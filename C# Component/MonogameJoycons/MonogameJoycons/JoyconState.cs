using System;
using System.Collections.Generic;

namespace Microsoft.Xna.Framework.Input
{
    public struct JoyconState
    { 
        public static readonly JoyconState Default = new JoyconState();
        //private UInt16 buttons;
        //private sbyte dstick;
        //private byte battery;
        //private bool isConnected;

        /**function of isButtonPressed
         * I have a uint16. 
         * 
         * so I could just store the uint and check for if the press is worked
         * BUT 
         * I need to be able to use the enum.
         * what if I just used the (int)key for the thing, and cut out the whole thing monogame does.
         * actually what does monogame do. 
         * 
         * */
        public bool isConnected { get; internal set; }

        private JoyconButtons buttons;
        //private JoyconStick stick;
        //private JoyconGyrescope gyrescope;
        //private JoyconAccelerometer accelerometer;

        public bool isButtonDown(JButtons button)
        {
            //Console.WriteLine("buttonDown flags: {0}",buttons._buttons);
            //Console.WriteLine("buttonDown button: {0}",button);
            //Console.WriteLine("buttonDown out: {0}", (buttons._buttons & button) == button);

            return (buttons._buttons & button) == button;
        }

        public bool isButtonUp(JButtons button)
        {
           
            return (buttons._buttons & button) != button;
        }

        public JoyconState(JoyconButtons buttons/*, JoyconStick joyconStick, JoyconGyrescope joyconGyrescope, JoyconAccelerometer joyconAccelerometer */ )
        {
            this.buttons = buttons;
            //stick = joyconStick;
            //gyrescope = joyconGyrescope;
            //accelerometer = joyconAccelerometer;
            isConnected = true;

        }


    }

    [Flags]
    public enum JoyconFlags
    {
        Y = 1,//right
        X = 2,//right
        B = 4,//right
        A = 8,//right
        Down = 1,//left
        Up = 2,//left
        Right = 4,//left
        Left = 8,//left
        SR = 16,//Both
        SL = 32,//Both
        R = 64,//Right
        L = 64,//Left 
        ZR = 128,//Right
        ZL = 128,//Left
        Plus = 512,//Right
        Minus = 256,//Left        
        LStick = 2048,//Left
        RStick = 1024,//Right
        Home = 4096,
        Capture = 8192

    }
    //so I'm worried that the left and right flags that share bytes, that if I check for a button on a left or right controller for the wrong one it'll return a false positive. 
    //etc left hold con is holding DOWN but checks for Y and returns true, because both of those check the same flag
    // so some things both joycons are tested in isolation. 
    //so I don't NEED to check for all teh different numbers, I just need a button for each joycon. then convert the existing flags into new flags
    //so I don't need two sets of triggers I just need one, I don't need two sticks. I do need plus and minus becasue they are distinct buttons. I only need one Sl SR because theyre is one on each and I don't need two. I don't need
    //while I don't need seperate triggers I should have them anyway because the Programmer isn't gonna care
    //dooo I? do *I* want that.
    //No, No I don't want to keep up with triggers on each ... if I do I should have differetn Sl and SR
    // you can just externally remember left and right.
    //I want to have both >:/
    [Flags]
    public enum JButtons
    {
        Y = 1<<0,        
        X = 1<<1,
        B = 1<<2,
        A = 1<<3,
        Down = 1<<4,
        Up = 1<<5,
        Right = 1<<6,
        Left = 1<<7,
        Trigger = 1<<8,        
        ZTrigger = 1<<9,
        SR = 1<<10,
        SL = 1<<11,
        Stick = 1<<12,        
        Plus = 1<<13,
        Minus = 1<<14,
        Home = 1<<15,
        Capture = 1<<16 
         
    }
    
}
