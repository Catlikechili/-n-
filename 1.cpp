#include "TrinamicStepper_windown.h"

int main() {
    TrinamicStepper stepper;
    stepper.init("COM9", CBR_9600); 
    stepper.close();
    if (!stepper.init("COM9", CBR_38400))
        return 1;

    stepper.moveTo(1, 0, 0, false);
    // stepper.rotateLeft(1,20,false);
    //  stepper.rotateLeft(1,200,false);
    // stepper.stop(1,false);
    stepper.close();
    return 0;
}