
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using USBInterface;
using System.Runtime.InteropServices;

namespace Microsoft.Xna.Framework.Input
{
    public static class Joycon
    {
        [DllImport("hidapi")]
        private static extern int joyconCount();
        [DllImport("hidapi")]
        private static extern IntPtr get_device(int device);
        [DllImport("hidapi")]
        private static extern void init_bt(IntPtr handle, int reportMode, int left_right, JoyconCalibration callibration);
        [DllImport("hidapi")]
        private static extern void handle_input(IntPtr handle, IntPtr state, int left_right);
        [DllImport("hidapi")]
        private static extern void rumble(IntPtr handle, int left_right, int frequency, int intensity);

        private class GamePadInfo
        {
            public IntPtr Device;      
            
        }
        private static readonly Dictionary<int, GamePadInfo> Gamepads = new Dictionary<int, GamePadInfo>();

        


        public static void JoyInit()
        {
            int numJoycons = joyconCount();
            for(int i = 0;i<numJoycons;i++)
            {
                GamePadInfo buildInfo = new GamePadInfo();
                buildInfo.Device = get_device(i);
                Gamepads.Add(i, buildInfo);
            }

        }
        public static JoyconState GetState()
        {
            JoyconState internalState = JoyconState.Default;

            return new JoyconState();
        }
        public static bool SetVibration()
        {
            return false;
        }

    }
}
