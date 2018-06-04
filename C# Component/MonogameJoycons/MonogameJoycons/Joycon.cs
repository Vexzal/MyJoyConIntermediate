
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using USBInterface;
using System.Runtime.InteropServices;

namespace Microsoft.Xna.Framework.Input
{
    /**
     * Todo
     * Buttons:
     *      Add functions to turn the uint16 from input into enum flags; (from memory its just check the uint against pressed or not then add them with |= to buttons, check the xinput getState
     * 
     * Gyrescope - Figure out extended functions, get a better container for incoming state;
     * Accelerometer - same as gyrescope.
     * Gyrescope - get a rot matrix
     * Acclerometer - get Transform matrix.
     * Sticks 
     *  see if you can just return a vector 2 to pass to the stick state constructor
     *      
     * */
    public static class Joycon
    {
        //initialize
        [DllImport("hidapi.dll")]
        private static extern int joyconCount();
        [DllImport("hidapi.dll",CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr get_device(int device);
        [DllImport("hidapi.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void init_bt(IntPtr handle, int reportMode, int left_right);
        [DllImport("hidapi.dll", CallingConvention = CallingConvention.Cdecl)]        
        private static extern int get_left_right(int device);
        [DllImport("hidapi.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr get_callibration(IntPtr handle, int left_right);
        [DllImport("hidapi.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void init_bt(IntPtr handle, int reportMode, int left_right, JoyconCalibration callibration);
        //input
        [DllImport("hidapi.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void handle_input(IntPtr handle, IntPtr state, int left_right);//outdated
        [DllImport("hidapi.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr get_packet(IntPtr handle);
        [DllImport("hidapi.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern UInt16 get_buttons(IntPtr handle, IntPtr packet, int left_right);
        [DllImport("hidapi.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Vector2 get_joyStick(IntPtr handle, IntPtr calibration, IntPtr packet, int left_right);
        [DllImport("hidapi.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern Vector3 get_accelerometer(IntPtr handle, IntPtr calibration, IntPtr packet, int left_right);
        [DllImport("hidapi.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern PairVector3 get_gyrescope(IntPtr handle, IntPtr calibration, IntPtr packet, ref int n, int left_right);
        //outFeatures
        [DllImport("hidapi.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern void rumble(IntPtr handle, int left_right, int frequency, int intensity);
        

        private class GamePadInfo
        {
            public IntPtr Device;
            public IntPtr Callibration;
            public int left_right;            
        }
        private static readonly Dictionary<int, GamePadInfo> Gamepads = new Dictionary<int, GamePadInfo>();

        


        public static void JoyInit()
        {
            int numJoycons = joyconCount();
            System.Console.WriteLine(numJoycons);
            for(int i = 0;i<numJoycons;i++)
            {
                Console.WriteLine(i);
                GamePadInfo buildInfo = new GamePadInfo();

                buildInfo.Device = get_device(i);

                buildInfo.left_right = get_left_right(i);
                Console.WriteLine("Left Right {0}", buildInfo.left_right);
                init_bt(buildInfo.Device, 0x30, buildInfo.left_right);

                buildInfo.Callibration = get_callibration(buildInfo.Device, buildInfo.left_right);

                Gamepads.Add(i, buildInfo);
                
            }

        }

        public static int LeftRight(PlayerIndex index)
        {
            return LeftRight((int)index);
        }

        internal static int LeftRight(int index)
        {
            return Gamepads[index].left_right;
        }

        public static JoyconState GetState(PlayerIndex index)
        {
            return GetState((int)index);
        }

        internal static JoyconState GetState(int index )
        {
            JoyconState internalState = JoyconState.Default;
            IntPtr packet = get_packet(Gamepads[index].Device);

            var gamepad = Gamepads[index];

            //UInt16 buttonStates = get_buttons(gamepad.Device, packet, gamepad.left_right);
            
            UInt16 zeroStates = 0;
            var buttons = ConvertToButtons(zeroStates,gamepad.left_right);

            // var stick = new JoyconStick(get_joyStick(
            //    gamepad.Device,
            //    gamepad.Callibration, 
            //    packet,
            //    Gamepads[index].left_right));
            //var gyrescope = (gamepad.left_right==1)?                         
            //    new JoyconGyrescope(
            //        get_gyrescope(Gamepads[index].Device,
            //        Gamepads[index].Callibration,
            //        packet,
            //        ref JoyconGyrescope.Offset.n_left,
            //        Gamepads[index].left_right))            
            //    :            
            //    new JoyconGyrescope(
            //        get_gyrescope(gamepad.Device,
            //        gamepad.Callibration,
            //        packet,
            //        ref JoyconGyrescope.Offset.n_right,
            //        gamepad.left_right));
            
            //var accelerometer = new JoyconAccelerometer(get_accelerometer
            //    (gamepad.Device,
            //    gamepad.Callibration,
            //    packet,
            //    gamepad.left_right));

            return new JoyconState(buttons/*, stick, gyrescope, accelerometer */);
        }
        private static JButtons AddButtonIfPressed(
            UInt16 buttonFlags, JoyconFlags joyconButton, JButtons xnaButton)
        {

            //Console.WriteLine("AddButton: buttonflags: {0}", buttonFlags);
            //Console.WriteLine("AddButton: joyconButton: {0}", (UInt16)joyconButton);
            //Console.WriteLine("AddButton: bit and: {0}", buttonFlags & (UInt16)joyconButton);

            var buttonState = ((buttonFlags & (UInt16)joyconButton) == (UInt16)joyconButton) ? ButtonState.Pressed : ButtonState.Released;
            return buttonState == ButtonState.Pressed ? xnaButton : 0;
        }
        
        private static JoyconButtons ConvertToButtons(UInt16 buttonFlags,int left_right)
        {
            
            var ret = (JButtons)0;
            if(left_right == 1)
            {
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.Up, JButtons.Up);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.Down, JButtons.Down);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.Left, JButtons.Left);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.Right, JButtons.Right);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.SR, JButtons.SR);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.SL, JButtons.SL);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.L, JButtons.Trigger);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.ZL, JButtons.ZTrigger);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.Minus, JButtons.Minus);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.LStick, JButtons.Stick);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.Capture, JButtons.Capture);
            }
            if(left_right == 2)
            {
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.Y, JButtons.Y);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.X, JButtons.X);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.B, JButtons.B);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.A, JButtons.A);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.SR, JButtons.SR);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.SL, JButtons.SL);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.R, JButtons.Trigger);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.ZR, JButtons.ZTrigger);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.Plus, JButtons.Plus);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.RStick, JButtons.Stick);
                ret |= AddButtonIfPressed(buttonFlags, JoyconFlags.Home, JButtons.Home);
            }
            return new JoyconButtons(ret);
        }
        public static bool SetVibration()
        {
            return false;
        }

    }
    
}
