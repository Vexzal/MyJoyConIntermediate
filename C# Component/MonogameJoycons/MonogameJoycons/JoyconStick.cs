using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Xna.Framework.Input
{
    public struct JoyconStick
    {        
        float CalX;
        float CalY;

        public JoyconStick(Vector2 input)
        {
            CalX = input.X;
            CalY = input.Y;
        }
    }
}
