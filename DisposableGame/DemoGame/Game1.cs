using System;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;


namespace DemoGame
{
    /// <summary>
    /// This is the main type for your game.
    /// </summary>
    public class Game1 : Game
    {
        GraphicsDeviceManager graphics;
        SpriteBatch spriteBatch;
        GamePadState exampState = GamePad.GetState(PlayerIndex.One);

        SpriteFont debugFont;

        JoyconPair oldState;
        JoyconPair newState;
        
        public Game1()
        {
            
            graphics = new GraphicsDeviceManager(this);
            Content.RootDirectory = "Content";
        }

        /// <summary>
        /// Allows the game to perform any initialization it needs to before starting to run.
        /// This is where it can query for any required services and load any non-graphic
        /// related content.  Calling base.Initialize will enumerate through any components
        /// and initialize them as well.
        /// </summary>
        protected override void Initialize()
        {
            // TODO: Add your initialization logic here
            IsFixedTimeStep = false;
            TargetElapsedTime = TimeSpan.FromSeconds(1);
            Joycon.JoyInit();

            
            base.Initialize();
        }
        Color backGroundColor = Color.CornflowerBlue;
        /// <summary>
        /// LoadContent will be called once per game and is the place to load
        /// all of your content.
        /// </summary>
        protected override void LoadContent()
        {
            // Create a new SpriteBatch, which can be used to draw textures.
            spriteBatch = new SpriteBatch(GraphicsDevice);
            newState = new JoyconPair();
            debugFont = Content.Load<SpriteFont>("Debug");

            

            // TODO: use this.Content to load your game content here
        }

        /// <summary>
        /// UnloadContent will be called once per game and is the place to unload
        /// game-specific content.
        /// </summary>
        protected override void UnloadContent()
        {
            // TODO: Unload any non ContentManager content here
        }

        /// <summary>
        /// Allows the game to run logic such as updating the world,
        /// checking for collisions, gathering input, and playing audio.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Update(GameTime gameTime)
        {
            if (GamePad.GetState(PlayerIndex.One).Buttons.Back == ButtonState.Pressed || Keyboard.GetState().IsKeyDown(Keys.Escape))
                Exit();
            oldState = newState;
            newState.GetState();

            
            
            //if (newState.Left.isButtonDown(JButtons.Down) && oldState.Left.isButtonUp(JButtons.Down))
            //{
            //    //Console.WriteLine("isChanged");
            //    backGroundColor = Color.Red;
            //}
            //if (newState.Left.isButtonUp(JButtons.Down) && oldState.Left.isButtonDown(JButtons.Down))
            //{
            //    backGroundColor = Color.CornflowerBlue;
            //}

            // TODO: Add your update logic here

            base.Update(gameTime);
        }

        /// <summary>
        /// This is called when the game should draw itself.
        /// </summary>
        /// <param name="gameTime">Provides a snapshot of timing values.</param>
        protected override void Draw(GameTime gameTime)
        {
            GraphicsDevice.Clear(backGroundColor);
            float frameRate = 1 / (float)gameTime.ElapsedGameTime.TotalSeconds;
            frameRate = (float)Math.Truncate((double)frameRate * 100.0 / 100.0);
            spriteBatch.Begin();
            spriteBatch.DrawString(debugFont, "" + frameRate, Vector2.Zero, Color.LightGreen);
            spriteBatch.End();
            // TODO: Add your drawing code here
            
            base.Draw(gameTime);
        }
    }
}
