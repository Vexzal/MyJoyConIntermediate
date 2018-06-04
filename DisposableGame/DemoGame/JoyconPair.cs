using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Input;
namespace DemoGame
{
    public struct JoyconPair
    {
        PlayerIndex leftIndex;
        PlayerIndex rightIndex;

        public JoyconState Left { get; private set; }

        public JoyconState Right { get; private set; }

        public void GetState()
        {
            Left = Joycon.GetState(leftIndex);
            Right = Joycon.GetState(rightIndex);
        }

        public JoyconPair(int i)
        {
            leftIndex = PlayerIndex.One;
            rightIndex = PlayerIndex.Two;
            if (Joycon.LeftRight(PlayerIndex.One) == 1)
            {
                leftIndex = PlayerIndex.One;
                rightIndex = PlayerIndex.Two;
            }
            else if (Joycon.LeftRight(PlayerIndex.Two) == 1)
            {
                leftIndex = PlayerIndex.Two;
                rightIndex = PlayerIndex.One;
            }
            Left = JoyconState.Default;
            Right = JoyconState.Default;
        }
    }
}
