using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Xna.Framework.Input
{
    public struct JoyconButtons
    {
        internal readonly JButtons _buttons;
        #region ButtonStates
        public ButtonState A
        {
            get { return ((_buttons & JButtons.A) == JButtons.A) ? ButtonState.Pressed : ButtonState.Released; }
        }

        public ButtonState B
        {
            get { return ((_buttons & JButtons.B) == JButtons.B) ? ButtonState.Pressed : ButtonState.Released; }
        }
        public ButtonState X
        {
            get { return ((_buttons & JButtons.X) == JButtons.X) ? ButtonState.Pressed : ButtonState.Released; }
        }
        public ButtonState Y
        {
            get { return ((_buttons & JButtons.Y) == JButtons.Y) ? ButtonState.Pressed : ButtonState.Released; }
        }
        public ButtonState Up
        {
            get { return ((_buttons & JButtons.Up) == JButtons.Up) ? ButtonState.Pressed : ButtonState.Released; }
        }
        public ButtonState Left
        {
            get { return ((_buttons & JButtons.Left) == JButtons.Left) ? ButtonState.Pressed : ButtonState.Released; }
        }
        public ButtonState Right
        {
            get { return ((_buttons & JButtons.Right) == JButtons.Right) ? ButtonState.Pressed : ButtonState.Released; }
        }
        public ButtonState Down
        {
            get { return ((_buttons & JButtons.Down) == JButtons.Down) ? ButtonState.Pressed : ButtonState.Released; }
        }
        public ButtonState Plus
        {
            get { return ((_buttons & JButtons.Plus) == JButtons.Plus) ? ButtonState.Pressed : ButtonState.Released; }
        }
        public ButtonState Minus
        {
            get { return ((_buttons & JButtons.Minus) == JButtons.Minus) ? ButtonState.Pressed : ButtonState.Released; }
        }
        #endregion

        public JoyconButtons(JButtons buttons)
        {
            
            _buttons = buttons;
        }
        internal JoyconButtons(params JButtons[]buttons):this()
        {
            foreach (JButtons b in buttons)
                _buttons |= b;

        }

    }
}
