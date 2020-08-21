#pragma once

int pin_1n1;
int pin_1n2;
int pin_1n3;
int pin_1n4;

void init_stepper(int pin_1n1, int pin_1n2, int pin_1n3, int pin_1n4);
void step(int);
void setOutput(int out);

int bitRead(unsigned char b, int bitPos);
