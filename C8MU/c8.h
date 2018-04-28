#pragma once

void startChip();
void execute();
void reset();

void writeRegister(int id, uint8_t value);
uint8_t readRegister(int id);

extern bool emulationAlive;

struct Instruction {
	uint16_t instr;
	Instruction(uint16_t opcode);

	uint8_t getInstructionIdentifier();
	uint16_t getAddress();
	uint8_t get8bitImmediate();
	uint8_t get4bitImmediate();
	uint8_t getXRegisterIdentifier();
	uint8_t getYRegisterIdentifier();
};