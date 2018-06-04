using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Xna.Framework.Input
{
    public struct JoyconAccelerometer
    {
        public float lastX;
        public float lastY;
        public float lastZ;

        public float x;
        public float y;
        public float z;


        public JoyconAccelerometer(Vector3 input)
        {
            x = input.X;
            y = input.Y;
            z = input.Z;

            lastX = 0;
            lastY = 0;
            lastZ = 0;
        }
        public JoyconAccelerometer(JoyconAccelerometer accelerometer)
        {
            this = accelerometer;
        }
    }
}
