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
            int n;
            float pitch;
            float yaw;
            float roll;
        }Offset offset;
    }
}
