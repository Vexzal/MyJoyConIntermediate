using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Xna.Framework.Input
{
    public struct JoyconGyrescope
    {
        public float pitch;
        public float yaw;
        public float roll;

        public struct Offset
        {
            public int n;
            public float pitch;
            public float yaw;
            public float roll;
        }public Offset offset;
    }
}
