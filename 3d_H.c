#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

// Screen size
#define SCREEN_WIDTH 120
#define SCREEN_HEIGHT 40

// Light vector
#define L_X 0
#define L_Y 1
#define L_Z -1

// Rotation speeds (faster for more dynamic animation)
#define ROT_SPEED_A 0.27
#define ROT_SPEED_B 0.18

// Shading characters
const char *shading = ".,-~:;=!*#$@";

// Parametric torus surface (for donut) with normals and shading
void render_torus(float cx, float cy, float cz, float R, float r,
                  float A, float B,
                  char *output, float *zbuffer) {
    for (float theta = 0; theta < 2 * M_PI; theta += 0.07) { // Major circle
        for (float phi = 0; phi < 2 * M_PI; phi += 0.02) {   // Minor circle
            // Parametric torus
            float x = (R + r * cos(phi)) * cos(theta);
            float y = (R + r * cos(phi)) * sin(theta);
            float z = r * sin(phi);
            // Center at (cx, cy, cz)
            float px = x + cx;
            float py = y + cy;
            float pz = z + cz;

            // Partial derivatives for normal
            // dP/dtheta
            float dx_theta = -sin(theta) * (R + r * cos(phi));
            float dy_theta = cos(theta) * (R + r * cos(phi));
            float dz_theta = 0;
            // dP/dphi
            float dx_phi = -sin(phi) * cos(theta) * r;
            float dy_phi = -sin(phi) * sin(theta) * r;
            float dz_phi = cos(phi) * r;
            // Normal = cross(dP/dtheta, dP/dphi)
            float nx = dy_theta * dz_phi - dz_theta * dy_phi;
            float ny = dz_theta * dx_phi - dx_theta * dz_phi;
            float nz = dx_theta * dy_phi - dy_theta * dx_phi;
            // Normalize normal
            float norm = sqrt(nx * nx + ny * ny + nz * nz);
            nx /= norm;
            ny /= norm;
            nz /= norm;

            // Rotate point
            float x_rot = px * cos(A) - py * sin(A);
            float y_rot = px * sin(A) + py * cos(A);
            float z_rot = pz;
            float y_rot2 = y_rot * cos(B) - z_rot * sin(B);
            float z_rot2 = y_rot * sin(B) + z_rot * cos(B);

            // Rotate normal
            float nx_rot = nx * cos(A) - ny * sin(A);
            float ny_rot = nx * sin(A) + ny * cos(A);
            float nz_rot = nz;
            float ny_rot2 = ny_rot * cos(B) - nz_rot * sin(B);
            float nz_rot2 = ny_rot * sin(B) + nz_rot * cos(B);

            // Lighting calculation
            float L_dot_N = nx_rot * L_X + ny_rot2 * L_Y + nz_rot2 * L_Z;
            int N = (int)((L_dot_N + 1) * 5.5); // Scale from 0 to 11
            if (N < 0) N = 0;
            if (N > 11) N = 11;

            // Projection
            float K1 = 80;
            float ooz = 1 / (z_rot2 + 40);
            int xp = (int)(SCREEN_WIDTH / 2 + K1 * ooz * x_rot);
            int yp = (int)(SCREEN_HEIGHT / 2 - K1 * ooz * y_rot2);
            int idx = xp + yp * SCREEN_WIDTH;
            if (xp >= 0 && xp < SCREEN_WIDTH && yp >= 0 && yp < SCREEN_HEIGHT) {
                if (ooz > zbuffer[idx]) {
                    zbuffer[idx] = ooz;
                    output[idx] = shading[N];
                }
            }
        }
    }
}

// Render parametric "H" using torus-like surfaces for legs and bar
void render_parametric_H(float A, float B, char *output, float *zbuffer) {
    // Left leg (vertical torus segment)
    for (float offset = -6; offset < 6; offset += 1.0) {
        render_torus(-5.0, offset, 0, 0.7, 0.5, A, B, output, zbuffer);
    }
    // Right leg (vertical torus segment)
    for (float offset = -6; offset < 6; offset += 1.0) {
        render_torus(5.0, offset, 0, 0.7, 0.5, A, B, output, zbuffer);
    }
    // Middle bar (horizontal torus segment)
    for (float offset = -4; offset < 5; offset += 0.8) {
        render_torus(offset, 0, 0, 0.7, 0.5, A, B, output, zbuffer);
    }
}



int main() {
    float A = 0, B = 0;
    char output[SCREEN_WIDTH * SCREEN_HEIGHT];
    float zbuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

    printf("\x1b[2J"); // Clear screen

    while (1) {
        memset(output, ' ', SCREEN_WIDTH * SCREEN_HEIGHT);
        memset(zbuffer, 0, sizeof(zbuffer));

        render_parametric_H(A, B, output, zbuffer);

        printf("\x1b[H");
        for (int k = 0; k < SCREEN_WIDTH * SCREEN_HEIGHT; k++) {
            putchar(k % SCREEN_WIDTH ? output[k] : '\n');
        }

        A += ROT_SPEED_A;
        B += ROT_SPEED_B;
        usleep(12000); // Decreased sleep for smoother and faster animation
    }

    return 0;
}
