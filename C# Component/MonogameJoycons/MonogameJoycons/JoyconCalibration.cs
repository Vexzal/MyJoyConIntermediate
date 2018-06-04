using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Microsoft.Xna.Framework.Input
{
    public struct JoyconCalibration
    {
        
        float[] acc_cal_coeff;
        float[] gyro_cal_coeff;
        float[] cal_x;
        float[] cal_y;

        bool hasUserCalStickL;
        bool hasUserCalStickR;
        bool hasUserCalSensor;

        byte[] factoryStickCal;
        byte[] userStickCal;
        byte[] sensorModel;
        byte[] stickModel;
    }
}
