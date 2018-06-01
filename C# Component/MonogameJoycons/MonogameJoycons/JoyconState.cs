using System;
using System.Collections.Generic;

namespace Microsoft.Xna.Framework.Input
{
    public struct JoyconState
    { 
        public static readonly JoyconState Default = new JoyconState();
        private UInt16 buttons;
        private sbyte dstick;
        private byte battery;
        private bool isConnected;

        private JoyconButtons bttnStates;
        private JoyconStick stick;
        private JoyconGyrescope gyrescope;
        private JoyconAccelerometer accelerometer;


    }

    [Flags]
    public enum JButtons
    {
        A,
        B,
        X,
        Y,
        Up,
        Down,
        Left,
        Right,
        L,
        R,
        ZL,
        ZR,
        Plus,
        Minus,
        SR,
        SL,
        LStick,
        RStick,
        Home,
        Capture

    }
}
