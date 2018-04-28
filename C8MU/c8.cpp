#include <stdint.h>
#include <stdlib.h> 
#include <stdio.h>
#include <time.h>
#include <GLFW/glfw3.h>
#include "c8.h"

#include "mem.h"
#include "display.h"

unsigned char chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

bool emulationAlive = true;
uint8_t registers[16];
uint16_t stack[16];
uint16_t addressRegister;
int pc;
int sp;

uint8_t delayTimer;
uint8_t soundTimer;
uint8_t timerCounter;

void startChip() {
	while (emulationAlive) {
		execute();
		pc += 2;
	}
}

void execute() {
	Instruction instruction = Instruction((ram[pc] << 8) | ram[pc + 1]);
	//printf("PC: 0x%04X\n", pc);
	pc += 2;
	switch (instruction.getInstructionIdentifier()) {
	case 0x0:
		switch (instruction.get8bitImmediate()) {
		case 0xE0:
			disp_clear();
			break;
		case 0xEE:
			pc = stack[--sp];
			break;
		default:
			printf("Undefined RCA Call instruction: 0x%04X", instruction.instr);
			emulationAlive = false;
			break;
		}
		break;
	case 0x1:
		pc = instruction.getAddress();
		break;
	case 0x2:
		stack[sp] = pc;
		sp++;
		pc = instruction.getAddress();
		break;
	case 0x3:
		if (readRegister(instruction.getXRegisterIdentifier()) == instruction.get8bitImmediate()) {
			pc += 2;
		}
		break;
	case 0x4:
		if (readRegister(instruction.getXRegisterIdentifier()) != instruction.get8bitImmediate()) {
			pc += 2;
		}
		break;
	case 0x5:
		if (readRegister(instruction.getXRegisterIdentifier()) == readRegister(instruction.getYRegisterIdentifier())) {
			pc += 2;
		}
		break;
	case 0x6:
		registers[instruction.getXRegisterIdentifier()] = instruction.get8bitImmediate();
		break;
	case 0x7:
		writeRegister(instruction.getXRegisterIdentifier(), readRegister(instruction.getXRegisterIdentifier()) + instruction.get8bitImmediate());
		break;
	case 0x8:
		switch (instruction.get4bitImmediate()) {
		case 0x0:
			writeRegister(instruction.getXRegisterIdentifier(), readRegister(instruction.getYRegisterIdentifier()));
			break;
		case 0x2:
			writeRegister(instruction.getXRegisterIdentifier(), readRegister(instruction.getXRegisterIdentifier()) & readRegister(instruction.getYRegisterIdentifier()));
			break;
		case 0x3:
			writeRegister(instruction.getXRegisterIdentifier(), readRegister(instruction.getXRegisterIdentifier()) ^ readRegister(instruction.getYRegisterIdentifier()));
			break;
		case 0x4:
			writeRegister(instruction.getXRegisterIdentifier(), readRegister(instruction.getXRegisterIdentifier()) + readRegister(instruction.getYRegisterIdentifier()));
			if (readRegister(instruction.getYRegisterIdentifier()) > readRegister(instruction.getXRegisterIdentifier())) {
				writeRegister(0xF, 1);
			} else {
				writeRegister(0xF, 0);
			}
			break;
		case 0x5:
			if (readRegister(instruction.getYRegisterIdentifier()) > readRegister(instruction.getXRegisterIdentifier())) {
				writeRegister(0xF, 0);
			} else {
				writeRegister(0xF, 1);
			}
			writeRegister(instruction.getXRegisterIdentifier(), readRegister(instruction.getXRegisterIdentifier()) - readRegister(instruction.getYRegisterIdentifier()));
			break;
		default:
			printf("Undefined instruction: 0x%04X", instruction.instr);
			emulationAlive = false;
			break;
		}
		break;
	case 0x9:
		if (readRegister(instruction.getXRegisterIdentifier()) != readRegister(instruction.getYRegisterIdentifier())) {
			pc += 2;
		}
		break;
	case 0xA:
		addressRegister = instruction.getAddress();
		break;
	case 0xC:
		writeRegister(instruction.getXRegisterIdentifier(), instruction.get8bitImmediate() & int((255 * rand()) / (RAND_MAX + 1.0)));
		break;
	case 0xD:
		draw_sprite(readRegister(instruction.getXRegisterIdentifier()), readRegister(instruction.getYRegisterIdentifier()), instruction.get4bitImmediate(), addressRegister);
		break;
	case 0xE:
		switch (instruction.get8bitImmediate()) {
		case 0x9E:
			// TODO: Skip if pressed
			break;
		case 0xA1:
			pc += 2; // TODO: Skip if not pressed
			break;
		default:
			printf("Undefined instruction: 0x%04X", instruction.instr);
			emulationAlive = false;
			break;
		}
		break;
	case 0xF:
		switch (instruction.get8bitImmediate()) {
		case 0x07:
			writeRegister(instruction.getXRegisterIdentifier(), delayTimer);
			break;
		case 0x15:
			delayTimer = readRegister(instruction.getXRegisterIdentifier());
			break;
		case 0x18:
			soundTimer = readRegister(instruction.getXRegisterIdentifier());
			break;
		case 0x1E:
			if ((addressRegister + readRegister(instruction.getXRegisterIdentifier()) > 0xFFF)) {
				writeRegister(0xF, 1);
			} else {
				writeRegister(0xF, 0);
			}
			addressRegister += readRegister(instruction.getXRegisterIdentifier());
			break;
		case 0x29:
			addressRegister = readRegister(instruction.getXRegisterIdentifier()) *0x5;
			break;
		case 0x33: {
			int bcd = readRegister(instruction.getXRegisterIdentifier());
			ram[addressRegister] = bcd / 100;
			ram[addressRegister + 1] = (bcd % 100) / 10;
			ram[addressRegister + 2] = (bcd % 100) % 10;
			break;
		}
		case 0x55:
			for (int i = 0; i <= instruction.getXRegisterIdentifier(); i++) {
				ram[addressRegister + i] = readRegister(i);
			}
			addressRegister += instruction.getXRegisterIdentifier() + 1;
			// Original interpreter: I = I + X + 1
			break;
		case 0x65:
			for (int i = 0; i < instruction.getXRegisterIdentifier(); i++) {
				writeRegister(i, ram[addressRegister + i]);
			}
			addressRegister += instruction.getXRegisterIdentifier() + 1;
			// Original interpreter: I = I + X + 1
			break;
		default:
			printf("Undefined instruction: 0x%04X", instruction.instr);
			emulationAlive = false;
			break;
		}
		break;
	default:
		printf("Undefined instruction: 0x%04X", instruction.instr);
		emulationAlive = false;
		break;
	}

	timerCounter++;
	if (timerCounter == 9) {
		timerCounter = 0;
		if (soundTimer > 0)
			soundTimer--;
		if (delayTimer > 0)
			delayTimer--;
	}
}

void reset() {
	pc = 0x200;
	sp = 0;
	for (int i = 0; i < 16; i++) {
		registers[i] = 0;
		stack[i] = 0;
	}
	// Load fonts into RAM
	for (int i = 0; i < 80; ++i) {
		ram[i] = chip8_fontset[i];
	}
	addressRegister = 0;
	disp_clear();
	
	delayTimer = 0;
	soundTimer = 0;
	timerCounter = 0;

	srand(time(NULL));
}

void writeRegister(int id, uint8_t value){
	registers[id] = value;
}

uint8_t readRegister(int id) {
	return registers[id];
}

Instruction::Instruction(uint16_t opcode) {
	instr = opcode;
}

uint8_t Instruction::getXRegisterIdentifier() {
	return (instr >> 8) & 0xF;
}

uint8_t Instruction::getYRegisterIdentifier() {
	return (instr >> 4) & 0xF;
}

uint8_t Instruction::get4bitImmediate() {
	return instr & 0xF;
}

uint8_t Instruction::get8bitImmediate() {
	return instr & 0xFF;
}

uint16_t Instruction::getAddress() {
	return instr & 0xFFF;
}

uint8_t Instruction::getInstructionIdentifier() {
	return instr >> 12;
}