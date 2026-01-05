/*
 * Madgwick AHRS Filter Implementation using CMSIS-DSP
 *
 * Copyright (C) 2025 - Derived work based on MadgwickAHRS
 * Original work: https://github.com/arduino-libraries/MadgwickAHRS
 * Algorithm: https://x-io.co.uk/open-source-imu-and-ahrs-algorithms/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This implementation replaces the external Madgwick library with native
 * Zephyr CMSIS-DSP functions for improved integration with embedded systems.
 */

#ifndef MADGWICK_CMSIS_H
#define MADGWICK_CMSIS_H

#include <arm_math.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

class MadgwickFilter {
private:
    float q0, q1, q2, q3;  // Quaternion elements (w, x, y, z)
    float beta;            // Filter gain
    float sample_freq;     // Sample frequency in Hz

    // Fast inverse square root
    float invSqrt(float x) {
        float halfx = 0.5f * x;
        float y = x;
        long i = *(long*)&y;
        i = 0x5f3759df - (i >> 1);
        y = *(float*)&i;
        y = y * (1.5f - (halfx * y * y));
        return y;
    }

public:
    MadgwickFilter() : q0(1.0f), q1(0.0f), q2(0.0f), q3(0.0f), beta(0.1f), sample_freq(100.0f) {}

    void begin(float freq) {
        sample_freq = freq;
        beta = 0.1f;  // Default gain
        q0 = 1.0f;
        q1 = 0.0f;
        q2 = 0.0f;
        q3 = 0.0f;
    }

    void setBeta(float b) {
        beta = b;
    }

    // IMU update (accelerometer + gyroscope only, no magnetometer)
    void updateIMU(float gx, float gy, float gz, float ax, float ay, float az) {
        float recipNorm;
        float s0, s1, s2, s3;
        float qDot1, qDot2, qDot3, qDot4;
        float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2, _8q1, _8q2, q0q0, q1q1, q2q2, q3q3;

        // Convert gyroscope degrees/sec to radians/sec
        gx *= 0.0174533f;
        gy *= 0.0174533f;
        gz *= 0.0174533f;

        // Rate of change of quaternion from gyroscope
        qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
        qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
        qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
        qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

        // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
        if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
            // Normalize accelerometer measurement
            recipNorm = invSqrt(ax * ax + ay * ay + az * az);
            ax *= recipNorm;
            ay *= recipNorm;
            az *= recipNorm;

            // Auxiliary variables to avoid repeated arithmetic
            _2q0 = 2.0f * q0;
            _2q1 = 2.0f * q1;
            _2q2 = 2.0f * q2;
            _2q3 = 2.0f * q3;
            _4q0 = 4.0f * q0;
            _4q1 = 4.0f * q1;
            _4q2 = 4.0f * q2;
            _8q1 = 8.0f * q1;
            _8q2 = 8.0f * q2;
            q0q0 = q0 * q0;
            q1q1 = q1 * q1;
            q2q2 = q2 * q2;
            q3q3 = q3 * q3;

            // Gradient descent algorithm corrective step
            s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
            s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
            s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
            s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;

            recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
            s0 *= recipNorm;
            s1 *= recipNorm;
            s2 *= recipNorm;
            s3 *= recipNorm;

            // Apply feedback step
            qDot1 -= beta * s0;
            qDot2 -= beta * s1;
            qDot3 -= beta * s2;
            qDot4 -= beta * s3;
        }

        // Integrate rate of change of quaternion to yield quaternion
        q0 += qDot1 * (1.0f / sample_freq);
        q1 += qDot2 * (1.0f / sample_freq);
        q2 += qDot3 * (1.0f / sample_freq);
        q3 += qDot4 * (1.0f / sample_freq);

        // Normalize quaternion using CMSIS-DSP
        float quat[4] = {q0, q1, q2, q3};
        arm_quaternion_normalize_f32(quat, quat, 1);
        q0 = quat[0];
        q1 = quat[1];
        q2 = quat[2];
        q3 = quat[3];
    }

    // Get roll in degrees
    float getRoll() {
        return getRollRadians() * 57.29578f;  // Convert to degrees
    }

    // Get pitch in degrees
    float getPitch() {
        return getPitchRadians() * 57.29578f;
    }

    // Get yaw in degrees
    float getYaw() {
        return getYawRadians() * 57.29578f;
    }

    // Get roll in radians
    float getRollRadians() {
        return atan2f(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2));
    }

    // Get pitch in radians
    float getPitchRadians() {
        float sinp = 2.0f * (q0 * q2 - q3 * q1);
        if (fabsf(sinp) >= 1.0f)
            return copysignf(M_PI / 2.0f, sinp); // Use 90 degrees if out of range
        else
            return asinf(sinp);
    }

    // Get yaw in radians
    float getYawRadians() {
        return atan2f(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3));
    }

    // Get quaternion components
    void getQuaternion(float *w, float *x, float *y, float *z) {
        *w = q0;
        *x = q1;
        *y = q2;
        *z = q3;
    }
};

#endif // MADGWICK_CMSIS_H
