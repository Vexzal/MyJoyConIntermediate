using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Xna.Framework.Input
{
    public struct JoyconGyrescope
    {
        public float roll;
        public float pitch;
        public float yaw;

        public struct Offset
        {
            public static int n_left = 0;
            public static int n_right = 0;
            internal float roll;
            internal float pitch;
            internal float yaw;
            
        }Offset offset;

        public JoyconGyrescope(JoyconGyrescope gyrescope)
        {
            this = gyrescope;
        }
        public JoyconGyrescope(PairVector3 input)
        {
            roll = input.A.X;
            pitch = input.A.Y;
            yaw = input.A.Z;
            offset = new Offset();
            offset.roll = input.B.X;
            offset.pitch = input.B.Y;
            offset.yaw = input.B.Z;
        }
    }

    public struct PairVector3
    {
        public Vector3 A;
        public Vector3 B;
    }
}
