#include "Mapper_01.h"

/*
 * Mapper 01, or MBC1
 * Max ROM : 2 MiB
 * Max RAM : 32 KiB
 *
 * In the default config MBC 1 supports up tp 512 KiB ROM with up to
 * 32 KiB RAM. The MBC can be wired differently where the 2-bit RAM
 * banking register is wired as an extension of the ROM banking register
 * allowing up to 2 MiB ROM. This is at the cost of only supporting a fixed
 * 8 KiB of RAM. All cartridges with 1 MiB ROM or more use this alternative
 * wiring.
 *
 * Memory range 0x0000 - 0x7fff is used for both reading and writing to
 * MBC control registers.
 *
 * Memory
 *
 * 0x0000 - 0x3fff - ROM Bank X0 [read-only]
 * Normally this area contains the first 16 KiB of ROM. In 1 MiB and larger
 * cartridges, entering mode 1 (Banking Mode) allow the second 2-bit banking
 * register to apply reads from this region in addition ot the regular 0x4000 -
 * 0x7fff. (0x20/0x40/0x60 or 0x10/0x20/0x30)
 *
 * 0x4000 - 0x7fff - ROM Bank 01-7f [read-only]
 * This area contains the rest of the 16 KiB ROM banks. If the main 5-bit ROM banking register is 0,
 * it is read as if the bank is set to 1.
 *
 * For 1 MiB+ ROM, any bank that is accessible via the 0x0000 - 0x3fff region is not accessible in this region.
 * (banks 0x00/0x20/0x40/0x60 in regular large ROM carts, or banks 0x00/0x10/0x20/0x30 in MBC1M multi-game compilation
 * carts. Instead, it is automatically mapped 1 bank higher.)
 * Note: bank 0x00 is always mapped to 0x0000 - 0x3fff.
 *
 * 0xa000 - 0xbfff - RAM Bank 00-03, if any
 *
 * This region is used with external RAM in the cartridge. RAM is only accessible if enabled, otherwise reads return
 * open bus values, and writes are ignored.
 *
 * Available RAM sizes are 8 KiB (0xa000 - 0xbfff) and 32 KiB (in the form of 4 8k banks). 32 KiB only available
 * in cartridges with ROM <= 512 KiB.
 *
 * External RAM is often battery-backed, allowing for game data to be sored when the Game Boy is off, or if the
 * cartridge is removed.
 *
 * Registers
 *
 * Initialized to 0x00, and "ROM Bank Number" register is treated as 0x01.
 *
 * 0x0000 - 0x1fff - RAM Enable (Write Only)
 * For RAM to be read or written to, the RAM Enable register must be set to 0x0a. Note that
 * any write to this register will have it's high nibble ignored, meaning that any
 * byte with the low nibble set to "a" will enable RAM. ie writing 0xea = 0x0a
 *
 * It is recommended to disable RAM after accessing it to protect it from corruption during GB power down,
 * or when the cartridge is removed from the system. RAM is automatically disabled when the cartridge loses power.
 *
 * 0x2000 - 0x3fff - ROM Bank Number (Write Only)
 *
 * This register is 5-bit (range 0x01-0x1f) and selects the ROM bank number for the 0x4000 - 0x7fff region of ROM.
 * Writes to this register will only retain the lower 5-bits. The upper 3 are discarded.
 *
 * When the register is set to 0x00, the bank will act as if it is set to 0x01. This prevents mirroring bank 0x00
 * in this region of ROM (There is a way to mirror the bank 0x00 on carts with 256 KiB ROM or less. Talked about
 * later in this section).
 *
 * When the bank register is set to a value higher than the number of banks in the cartridge, then the number
 * is masked to the required number of bits. e.x a 256 KiB cart only needs a 4-bit bank number to address all of its
 * 16 banks, so the upper bit is ignored in this case.
 *
 * Regardless of the number of banks, the 5-bits are used for the bank translation logic (changing the bank number
 * when it is 0x00 -> 0x01). This means that in cartridges where the ROM size is 256 KiB or smaller, a programmer can
 * write the value 0x10 or 16 to this register, and bank 0x00 would be mirrored in this region.
 *
 * On carts that need more than a 5-bit bank number, the secondary 2 bit banking register at 0x4000 - 0x5fff is used
 * to extend the range and provide 2 additional bits.
 *
 * Formula for NON MBC1M carts: Selected ROM bank = (Secondary Bank Number << 5) + ROM Bank Number
 *
 * The additional 2 bits are ignored for the bank 0x00->0x01 translation. This causes problems accessing banks
 * 0x20, 0x40, and 0x60 as these values only set bits in the upper 2-bit register. As only the lower 5-bits
 * are used in the 0x00->0x01 bank translation, this will cause the banks 0x21, 0x41, and 0x61 to be accessed instead.
 * The only way to access these banks is to switch to mode 1 which remaps the 0x0000-0x3fff region.
 *
 * 0x4000 - 0x5fff - RAM Bank Number - or - Upper Bits if ROM Bank Number (Write Only)
 *
 * This 2-bit register can be used to select a RAM bank in range 0x00-0x03, or it is used to specify bits 5-6 in > 1MiB
 * cartridges. If neither ROM nor RAM are large enough this register does nothing.
 *
 * In 1MiB MBC1 multi-carts, this register is mapped to bits 4-5 of the ROM bank number and the top bit of the main
 * 5-bit ROM banking register is ignored.
 *
 * 0x6000 - 0x7fff - Banking Mode Select (Write Only)
 *
 * A 1-bit register used to control the behavior of the 2-bit banking register. If the cart is not large enough to
 * use the 2-bit register (<= 8 KiB RAM and <= 512 KiB ROM) this register has no observable effect.
 * 0x00 = Simple banking (defualt)
 *        0x0000-0x3fff and 0xa000-0xbfff locked to bank 0 of ROM/RAM
 * 0x01 = RAM banking mode / Advance ROM banking mode
 *
 * Note: Addressing diagrams are available here: https://gbdev.io/pandocs/MBC1.html
 *       We will be using a 21-bit address to access all 2 MiB of ROM, and a 15-bit address for all 32 KiB of RAM.
 *       We can mask the address depending on if the cart has the according ROM/RAM or not.
 */

Mapper_01::Mapper_01(uint8_t rom_banks, uint8_t ram_banks) : Mapper(rom_banks, ram_banks) {
    reg.data = 0; addresses.rom_address = 0; addresses.ram_address = 0;
}
// TODO(kaelan): Review how this MBC works. Also should probably rename the classes to MBC rather than Mapper...
bool Mapper_01::cpu_map_read(uint16_t addr, uint32_t &mapper_addr, bool &ram) {
    // Check if the address is within cartridge ROM
    if (addr >= 0x0000 && addr <= 0x7fff) {
        // First check if the address is in range 0x0000-0x3fff: ROM0
        if (addr >= 0x0000 && addr <= 0x3fff) {
            // Now check to see if we are in mode 0
            if (reg.bank_mode == 0) {
                // In this mode we should be able to access the address directly, no
                // mapping required.
                mapper_addr = addr;
            }
            else {
                // In mode 1, we will need to check a few things
                // First see if we have <= 32 ROM banks
                if (rom_banks <= 0x20) {
                    // In this case we use mode 1 address mapping
                    mapper_addr = addr;
                    /* Disable bits 14-20: We don't have more than 32 banks
                     * so, we can ignore the upper 2-bit register, effectively making this
                     * the same as mode 0.
                     */
                    mapper_addr &= ~(0x1fc000);
                }
                else {
                    mapper_addr = addr;
                    // Disable bits 14-18
                    mapper_addr &= ~(0x7c000);
                    // OR the address with the 2-bit register
                    mapper_addr |= (reg.ram_rom_2bit << 19);
                }
            }
        }
        // Now check if we are writing to 0x4000-0x7fff
        // TODO(kaelan): Look more closely into how this works! I believe that this is the only ROM area that
        //               we need to worry about while looking at Zelda.
        else if (addr >= 0x4000 && addr <= 0x7fff) {
            // Check if banks <= 0x20
            if (rom_banks <= 0x20) {
                // In this case, we ignore the 2-bit register
                mapper_addr = addr & 0x3fff;
                //if (reg.rom_bank_number == 2) {

                //}
                // Shift the bank number into mapper_addr 14 bits
                mapper_addr |= (reg.rom_bank_number << 14);
                // Disable bits 19 & 20
                mapper_addr &= ~(0x180000);
            }
            else {
                // Apply the full address mapping
                mapper_addr = addr & 0x3fff;
                mapper_addr |= (reg.rom_bank_number << 14);
                mapper_addr |= (reg.ram_rom_2bit    << 19);
            }
        }
        return true;
    }
    // Check if the address is within cartridge RAM
    else if (addr >= 0xa000 && addr <= 0xbfff) {
        // Check if RAM is enabled
        if (reg.ram_enable) {
            // Check bank mode and that there is more than 1 bank
            if (reg.bank_mode == 0x1 && ram_banks > 1) {
                mapper_addr = addr & 0x1fff;
                // Shift in the 2-bit register 13 to the left
                mapper_addr |= (reg.ram_rom_2bit << 13);
                ram = true;
            }
            else {
                // If we get here, there is no need to map the address
                mapper_addr = addr & 0x1fff;
                ram = true;
            }
            return true;
        }
        else {
            //printf("Mapper_01 Error: Attempted to access address: 0x%x. RAM not Enabled.\n", addr);
            return false;
        }
    }
    return false;
}
// TODO(kaelan): Add a mapped_addr out param so that we can write to RAM!!!!!
bool Mapper_01::cpu_map_write(uint16_t addr, uint8_t data, uint32_t &mapped_addr, bool &ram) {
    /* MBC01 will handle the following address ranges:
     * 0x0000-0x1fff RAM Enable
     * 0x2000-0x3fff ROM Bank Number
     * 0x4000-0x5fff RAM Bank Number/Upper ROM Bank Number (Depending on ROM size)
     * 0x6000-0x7fff Banking mode
     */

    // Check for RAM enable register write
    if (addr >= 0x0000 && addr <= 0x1fff) {
        // Check if 0x0a is in the low nibble of data - this is the value that needs to be written to the
        // ram_enable register for RAM to be enabled.
        if ((data & 0x0f) == 0x0a) {
            reg.ram_enable = 1;
            //printf("RAM Enabled.\n");
        }
        else {
            reg.ram_enable = 0;
            //printf("RAM Disabled.\n");
        }
        return true;
    }
    // Check for ROM Bank Number write
    else if (addr >= 0x2000 && addr <= 0x3fff) {
        // Apply bit mask and check if the value is 0
        if ((data & 0x1f) == 0x00)
            reg.rom_bank_number = 0x01;
        else
            reg.rom_bank_number = data & 0x1f;
        return true;
    }
    // Check for RAM/upper ROM 2-bit register write
    else if (addr >= 0x4000 && addr <= 0x5fff) {
        // Store using bit mask - 2-bit
        reg.ram_rom_2bit = data & 0b11;
    }
    // Check for bank mode register write
    else if (addr >= 0x6000 && addr <= 0x7fff) {
        // Store using bit mask - 1-bit
        reg.bank_mode = data & 0b1;
    }
    // Now check for a RAM write
    // TODO(kaelan): Actually write to RAM!!!!
    else if (addr >= 0xa000 && addr <= 0xbfff) {
        // Now check if RAM is enabled
        if (reg.ram_enable == 1) 
        {
            ram = true;
            if(reg.bank_mode == 0)
            {

                mapped_addr = addr & 0x1fff;
            }
            else
            {
                mapped_addr = addr & 0x1fff;
                mapped_addr |= reg.ram_rom_2bit << 13;
            }
            return true;
        }
        else {
            printf("Mapper_01 Error: Attempt to write to address: 0x%x when RAM is disabled\n", addr);
        }
    }
    else {
        //printf("Mapper_01 Error: Attempted write to address: 0x%x. Address not writeable.\n", addr);
        //return false;
    }
    return false;
}