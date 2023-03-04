/*
 * SRAM_23LC.h
 *
 *  Created on: Feb 28, 2023
 *      Author: torak.david
 */

#ifndef SRAM_23XXXXX_SRAM_23LC_H_
#define SRAM_23XXXXX_SRAM_23LC_H_

#include "MojeFunkce.h"
#include "MojeMacro.h"

typedef enum _SRAM_23LC_Device
{
// 128KB
	SRAM_23LCV1024 = 0,
	SRAM_23LC1024,
	SRAM_23A1024,
// 64KB
	SRAM_23LCV512,
	SRAM_23LC512,
	SRAM_23A512,
// 32KB
	SRAM_23A256,
	SRAM_23K256,
// 8KB
	SRAM_23A640,
	SRAM_23K640,
} SRAM_23LC_Device;

#define SRAM_CAPACITY_23LCV1024                 0x20000
#define SRAM_CAPACITY_23LC1024                  0x20000
#define SRAM_CAPACITY_23A1024                   0x20000
#define SRAM_CAPACITY_23LCV512                  0x10000
#define SRAM_CAPACITY_23LC512                   0x10000
#define SRAM_CAPACITY_23A512                    0x10000
#define SRAM_CAPACITY_23A256                    0x8000
#define SRAM_CAPACITY_23K256                    0x8000
#define SRAM_CAPACITY_23A640                    0x2000
#define SRAM_CAPACITY_23K640                    0x2000


#define SRAM_PAGE_23K256                    32		//1 stránka 32 bytů
#define SRAM_PAGES_23K256					1024 	// počet stránek 1024

#define SRAM_23LC_ADDRESS_16BIT			0
#define SRAM_23LC_ADDRESS_24BIT			1

#define SRAM_23LC_DUMMY_BYTE			0xFF

// Příkazy podporované všemi čipy
#define SRAM_23LC_COMMAND_READ			0x03
#define SRAM_23LC_COMMAND_WRITE			0x02
#define SRAM_23LC_COMMAND_RDMR			0x05
#define SRAM_23LC_COMMAND_WRMR			0x01

// Další příkazy podporované některými čipy
#define SRAM_23LC_COMMAND_EDIO			0x3B
#define SRAM_23LC_COMMAND_EQIO			0x38
#define SRAM_23LC_COMMAND_RSTIO			0xFF


// nastavení STATUS registru
#define SRAM_23LC_STATUS_BITE			0
#define SRAM_23LC_STATUS_SEQUENTIAL		64
#define SRAM_23LC_STATUS_PAGE			128




typedef struct {
	GPIO_TypeDef *SS_port;
	u16 SS_pin;
	SPI_HandleTypeDef *hdspi;
	SRAM_23LC_Device device;
	u32 capacity;
	u8 pageByts;
	u16 pages;
} SRAM23_HandleTypeDef;

HAL_StatusTypeDef SRAM_23xx_init(SRAM23_HandleTypeDef *hdev, SPI_HandleTypeDef *hdspi, GPIO_TypeDef *SS_port, u16 SS_pin, SRAM_23LC_Device device);
size_t SRAM_23xx_writeByte(SRAM23_HandleTypeDef *hdev, const uint32_t address, const uint8_t byte_);
u8 SRAM_23xx_readByte(SRAM23_HandleTypeDef *hdev, const uint32_t address);
bool SRAM_23xx_test(SRAM23_HandleTypeDef *hdev);
size_t SRAM_23xx_writeBlock(SRAM23_HandleTypeDef *hdev, const uint32_t address, const size_t length, u8 *buffer);
size_t SRAM_23xx_readBlock(SRAM23_HandleTypeDef *hdev, const uint32_t address, const size_t length, u8 *buffer);

size_t SRAM_23xx_writePage(SRAM23_HandleTypeDef *hdev, const uint16_t page, size_t length, u8 *buffer);
size_t SRAM_23xx_readPage(SRAM23_HandleTypeDef *hdev, const uint16_t page, size_t length, u8 *buffer);














#endif /* SRAM_23XXXXX_SRAM_23LC_H_ */
