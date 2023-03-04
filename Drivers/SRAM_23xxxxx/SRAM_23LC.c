/*
 * SRAM_23LC.c
 *
 *  Created on: Feb 28, 2023
 *      Author: torak.david
 *  https://www.digikey.cz/en/maker/projects/getting-started-with-stm32-how-to-use-spi/09eab3dfe74c4d0391aaaa99b0a8ee17
 */

#include "SRAM_23LC.h"
#include "MojeFunkce.h"
#include "MojeMacro.h"

#define SPI_READYNESS_DELAY		500

static void activeSS_sram(SRAM23_HandleTypeDef *hdev);
static void deActiveSS_sram(SRAM23_HandleTypeDef *hdev);
static u8 getStatus_ram(SRAM23_HandleTypeDef *hdev);
static void setStatus_ram(SRAM23_HandleTypeDef *hdev, u8 status);
static u8 pripravData(SRAM23_HandleTypeDef *hdev, u32 address, u8 command, u8 *data);

/**
 * Přečte status registr
 * @param 	SRAM23_HandleTypeDef (&hsram)
 * @návrat 	hodnota regidtru
 */
static u8 getStatus_ram(SRAM23_HandleTypeDef *hdev) {
	u8 tmp = 69;
	activeSS_sram(hdev);// aktivuj slave select (0)
	HAL_SPI_TransmitReceive(hdev->hdspi, (u8*) SRAM_23LC_COMMAND_RDMR, &tmp, 1, SPI_READYNESS_DELAY);
	//HAL_SPI_Transmit(hdev->hdspi, (u8*) SRAM_23LC_COMMAND_RDMR, 1, SPI_READYNESS_DELAY);
	//HAL_SPI_Receive(hdev->hdspi, &tmp, 1, SPI_READYNESS_DELAY);
	deActiveSS_sram(hdev);// deaktivuj slave select (1)
	return tmp;
}

/**
 * Zápis so status registru
 * @param 	SRAM23_HandleTypeDef (&hsram)
 * @param 	hodnota
 * 				SRAM_23LC_STATUS_BITE
 *				SRAM_23LC_STATUS_SEQUENTIAL
 *				SRAM_23LC_STATUS_PAGE
 */
static void setStatus_ram(SRAM23_HandleTypeDef *hdev, u8 status) {
	activeSS_sram(hdev);// aktivuj slave select (0)
	u8 data[3];// příprava dat pro poslání SPI (co dělat a data)
	data[0] = SRAM_23LC_COMMAND_WRMR;
	data[1] = status;
	HAL_SPI_Transmit(hdev->hdspi, (u8*) data, 2, SPI_READYNESS_DELAY);
	deActiveSS_sram(hdev);// deaktivuj slave select (1)
}

/**
 * Aktivace Slave Select pro SRAM (0)
 * @param 	SRAM23_HandleTypeDef (&hsram)
 */
static void activeSS_sram(SRAM23_HandleTypeDef *hdev) {
	HAL_GPIO_WritePin(hdev->SS_port, hdev->SS_pin, LOW);
	//delay(2);

}

/**
 * Deaktivace Slave Select pro SRAM (1)
 * @param 	SRAM23_HandleTypeDef (&hsram)
 */
static void deActiveSS_sram(SRAM23_HandleTypeDef *hdev) {
	HAL_GPIO_WritePin(hdev->SS_port, hdev->SS_pin, HIGH);

}

/**
 * Příprava dat pro komunikaci s SRAM (co se bude dělat a adresa)
 * @param SRAM23_HandleTypeDef (&hsram)
 * @param adresa se kterou se bude pracovat
 * @param jaký je příkaz
 * @param data je buffer kam se vše uloží
 * @návrat velikost připravených dat
 */
static u8 pripravData(SRAM23_HandleTypeDef *hdev, u32 address, u8 command, u8 *data) {
	u8 size = 4; //prozatím velikost dat, pokud se nezmění, bude použita
	data[0] = command; // první je vždy příkaz
	if (hdev->capacity > 0x10000){// zde se řeší zda je adresa 24b nebo 16b podle velikosti SRAM
		// adresa je 24b
		data[1] = (uint8_t) ((address >> 16) & 0xFF);
		data[2] = (uint8_t) ((address >> 8) & 0xFF);
		data[3] = (uint8_t) ((address) & 0xFF);
	}else{
		// adresa je 16b
		data[1] = (uint8_t) ((address >> 8) & 0xFF);
		data[2] = (uint8_t) ((address) & 0xFF);
		size = 3;// protože je adresa pouze 2B, velikost dat bude 3 (command + 2B adresa)
	}
	return size;
}

/**
 * Prvotní inicializace SRAM
 * @param SRAM23_HandleTypeDef (&hsram)
 * @param spi
 * @param Gpio port SS
 * @param Gpio pin SS
 * @param typ zařízení SRAM_23LC_Device
 */
HAL_StatusTypeDef SRAM_23xx_init(SRAM23_HandleTypeDef *hdev, SPI_HandleTypeDef *hdspi, GPIO_TypeDef *SS_port, u16 SS_pin, SRAM_23LC_Device device) {
	u8 tmp = 69; // pomocná proměnná kam se načte status
	// do předané struktury se nastavý všechny parametry
	hdev->SS_pin = SS_pin;
	hdev->SS_port = SS_port;
	hdev->device = device;
	hdev->hdspi = hdspi;
	// na použíté paměti závisí její kapacita (maximální adresa)
	if ((device == SRAM_23LCV1024) || (device == SRAM_23LC1024) || (device == SRAM_23A1024)){
		hdev->capacity = 0x20000;
	}else if ((device == SRAM_23LCV512) || (device == SRAM_23LC512) || (device == SRAM_23A512)){
		hdev->capacity = 0x10000;
	}else if ((device == SRAM_23A256) || (device == SRAM_23K256)){
		hdev->capacity = 0x8000;
		hdev->pageByts = SRAM_PAGE_23K256;
		hdev->pages = SRAM_PAGES_23K256;
	}else if ((device == SRAM_23A640) || (device == SRAM_23K640)){
		hdev->capacity = 0x2000;
	}
	tmp = getStatus_ram(hdev);
	printf("STATUS_INIT_PRED: %d\r\n", tmp);
	setStatus_ram(hdev, SRAM_23LC_STATUS_BITE);
	tmp = getStatus_ram(hdev);
	printf("STATUS_INIT_PO: %d\r\n", tmp);
	return HAL_OK;
}

/**
 * Zápis jednoho Bajtu na adresu
 * @param SRAM23_HandleTypeDef (&hsram)
 * @param adresa na kterou se bude ukládat
 * @param byte_ Bajt ktery se uloží
 * @návrat zatím nepodstatný
 */
size_t SRAM_23xx_writeByte(SRAM23_HandleTypeDef *hdev, const uint32_t address, const uint8_t byte_) {
	setStatus_ram(hdev, SRAM_23LC_STATUS_BITE);// status na práci s 1 Bajtem
	if (address >= hdev->capacity){// ochrana překročení adresy
		return (0);
	}
	u8 data[5];// zde bude hlavička poslaná SPI SRAM
	// vzhledem k univerzálnosti projistotu vyplníme obě poslední hodnoty hlavičky
	data[4] = byte_;
	data[3] = byte_;
	size_t size = pripravData(hdev, address, SRAM_23LC_COMMAND_WRITE, data);// připrav hlavičku a velikost hlavičky
	activeSS_sram(hdev);
	HAL_SPI_Transmit(hdev->hdspi, data, size, SPI_READYNESS_DELAY);
	deActiveSS_sram(hdev);
	return (1);
}

/**
 * Čtení jednoho Bajtu z adresy
 * @param SRAM23_HandleTypeDef (&hsram)
 * @param adresa ze které se bude číst
 * @návrat přečtený Bajt
 */
u8 SRAM_23xx_readByte(SRAM23_HandleTypeDef *hdev, const uint32_t address) {
	uint8_t ret = 69;	// zde se uloží zapsaná data
	setStatus_ram(hdev, SRAM_23LC_STATUS_BITE);// nastav status pro čtení jednoho Bajtu
	if (address >= hdev->capacity){// ochrana překročení adresy
		return (0);
	}
	u8 data[4];// zde bude hlavička poslaná SPI SRAM
	size_t size = pripravData(hdev, address, SRAM_23LC_COMMAND_READ, data);// připrav hlavičku a velikost hlavičky
	activeSS_sram(hdev);
	HAL_SPI_Transmit(hdev->hdspi, data, size, SPI_READYNESS_DELAY);
	HAL_SPI_Receive(hdev->hdspi, &ret, 1, SPI_READYNESS_DELAY);
	deActiveSS_sram(hdev);
	return (ret);
}

/**
 * Zápis bloku od adresy
 * @param SRAM23_HandleTypeDef (&hsram)
 * @param adresa na kterou se začne  ukládat
 * @param length velikost ukládaných dat
 * @param data v buffru pro uložení
 * @návrat délka dat
 */
size_t SRAM_23xx_writeBlock(SRAM23_HandleTypeDef *hdev, const uint32_t address, const size_t length, u8 *buffer) {
	u8 data[4];// zde bude hlavička poslaná SPI SRAM
	setStatus_ram(hdev, SRAM_23LC_STATUS_SEQUENTIAL);// nastav status pro sekvenční čtení/zápis
	size_t size = pripravData(hdev, address, SRAM_23LC_COMMAND_WRITE, data);// připrav hlavičku a velikost hlavičky
	activeSS_sram(hdev);
	HAL_SPI_Transmit(hdev->hdspi, data, size, SPI_READYNESS_DELAY);
	HAL_SPI_Transmit(hdev->hdspi, buffer, length, SPI_READYNESS_DELAY);
	deActiveSS_sram(hdev);
	return length;
}

/**
 * Čtení bloku od adresy
 * @param SRAM23_HandleTypeDef (&hsram)
 * @param adresa ze které se začne  číst
 * @param length velikost čtených dat
 * @param buffr pro uložení přečtených dat
 * @návrat délka dat
 */
size_t SRAM_23xx_readBlock(SRAM23_HandleTypeDef *hdev, const uint32_t address, const size_t length, u8 *buffer) {
	u8 data[4];// zde bude hlavička poslaná SPI SRAM
	setStatus_ram(hdev, SRAM_23LC_STATUS_SEQUENTIAL);// nastav status pro sekvenční čtení/zápis
	size_t size = pripravData(hdev, address, SRAM_23LC_COMMAND_READ, data);// připrav hlavičku a velikost hlavičky
	activeSS_sram(hdev);
	HAL_SPI_Transmit(hdev->hdspi, data, size, SPI_READYNESS_DELAY);
	HAL_SPI_Receive(hdev->hdspi, buffer, length, SPI_READYNESS_DELAY);
	deActiveSS_sram(hdev);
	return length;
}

/**
 * Zápis na stránku
 * @param SRAM23_HandleTypeDef (&hsram)
 * @param stráánka do které se bude  ukládat
 * @param length velikost ukládaných dat
 * @param data v buffru pro uložení
 * @návrat délka dat
 */
size_t SRAM_23xx_writePage(SRAM23_HandleTypeDef *hdev, const uint16_t page, size_t length, u8 *buffer) {
	u8 data[4];// zde bude hlavička poslaná SPI SRAM
	if(length>hdev->pageByts-1) hdev->pageByts-1;// ochrana aby se nazapisovalo výc než je Bajtů v jedné stránce
	setStatus_ram(hdev, SRAM_23LC_STATUS_PAGE);// nastav status pro práci se stránkou čtení/zápis
	size_t size = pripravData(hdev, page, SRAM_23LC_COMMAND_WRITE, data);// připrav hlavičku a velikost hlavičky
	activeSS_sram(hdev);
	HAL_SPI_Transmit(hdev->hdspi, data, size, SPI_READYNESS_DELAY);
	HAL_SPI_Transmit(hdev->hdspi, buffer, length, SPI_READYNESS_DELAY);
	if(length<hdev->pageByts-1)// pokud je méně dat, odešle 0 do počtu
		HAL_SPI_Transmit(hdev->hdspi, (u8*)0, hdev->pageByts-1-length, SPI_READYNESS_DELAY);
	deActiveSS_sram(hdev);
	return length;
}

/**
 * Čtení ze stránky
 * @param SRAM23_HandleTypeDef (&hsram)
 * @param stráánka ze které se bude  číst
 * @param length velikost čtenych dat
 * @param buffr kde se uloží přečtená data
 * @návrat délka dat
 */
size_t SRAM_23xx_readPage(SRAM23_HandleTypeDef *hdev, const uint16_t page, size_t length, u8 *buffer) {
	u8 data[4];// zde bude hlavička poslaná SPI SRAM
	if(length>hdev->pageByts-1) hdev->pageByts-1;// ochrana aby se nazapisovalo výc než je Bajtů v jedné stránce
	setStatus_ram(hdev, SRAM_23LC_STATUS_PAGE);// nastav status pro práci se stránkou čtení/zápis
	size_t size = pripravData(hdev, page, SRAM_23LC_COMMAND_READ, data);// připrav hlavičku a velikost hlavičky
	activeSS_sram(hdev);
	HAL_SPI_Transmit(hdev->hdspi, data, size, SPI_READYNESS_DELAY);
	HAL_SPI_Receive(hdev->hdspi, buffer, length, SPI_READYNESS_DELAY);
	deActiveSS_sram(hdev);
	return length;
}

/**
 * test všech paměťových buněk (velmi dlouhé)
 * @param SRAM23_HandleTypeDef (&hsram)
 * @návrat zda vše proběhlo ok nebo byla detekovaná chyba
 */
bool SRAM_23xx_test(SRAM23_HandleTypeDef *hdev) {
	bool ret = true;
	u8 test;
	for (u16 b = 0; b < 256; b++){
		for (u16 a = 0; a < hdev->capacity; a++){
			SRAM_23xx_writeByte(hdev, a, b);
			//printf("%u - test RAM: A:%c\r\n", a, (char) test);
		}
		for (u16 a = 0; a < hdev->capacity; a++){
			test = b - 1;
			test = SRAM_23xx_readByte(hdev, a);
			if (test != b){
				return false;
			}
			//printf("%u - test RAM: A:%c\r\n", a, (char) test);
		}
		printf("OK: %d\r\n", b);
	}
	return ret;
}
