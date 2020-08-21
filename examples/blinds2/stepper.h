int lookup[8] = { 0b0001, 0b011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001 };
int motorSpeed = 1200;  //variable to set stepper speed
int countsperrev = 512; // number of steps per full revolution
int pin_1n1;
int pin_1n2;
int pin_1n3;
int pin_1n4;

void init_stepper(int pin_1n1, int pin_1n2, int pin_1n3, int pin_1n4);
void step(int);
void setOutput(int out);
