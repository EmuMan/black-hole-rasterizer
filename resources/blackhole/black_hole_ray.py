from scipy.integrate import solve_ivp, solve_bvp
import numpy as np
import matplotlib.pyplot as plt
from math import pi

# Constants
a = 1  # Setting a normalized Schwarzschild radius for simplicity

# System of equations for (t, r, phi, dt/ds, dr/ds, dphi/ds)
def geodesic_equations(_s, y):
    t, r, phi, dt, dr, dphi = y

    # Avoid division by zero near r = a
    if r <= a:
        r = a + 1e-6

    # Geodesic equations
    d2t = - (a / r**2) * (1 - a / r)**-1 * dt * dr
    d2r = - (1/2) * (1 - a / r) * (a * dt**2 / r**2 - a * dr**2 / (r - a)**2 - 2 * r * dphi**2)
    d2phi = - (2 * dr * dphi) / r

    return [dt, dr, dphi, d2t, d2r, d2phi]

# Initial conditions
r0 = 15 * a  # Initial radial distance
phi0 = -pi / 2     # Initial angle
t0 = 0       # Initial coordinate time

# Initial velocities (choosing parameters that show deflection)
dt0 = 1 / (1 - a / r0)  # Time dilation factor
dr0 = -1.0  # Approaching the black hole
dphi0 = 0.02 # Initial angular velocity

y0 = [t0, r0, phi0, dt0, dr0, dphi0]

# Integration range
s_span = (0, 100)  # Proper time-like parameter range

# Solve the system of ODEs
sol = solve_ivp(geodesic_equations, s_span, y0, method='RK45', t_eval=np.linspace(0, 50, 1000))

# Extract solutions
r_sol = sol.y[1]
phi_sol = sol.y[2]

print(r_sol)
print(phi_sol)

# Convert to Cartesian coordinates for visualization
x_sol = r_sol * np.cos(phi_sol)
y_sol = r_sol * np.sin(phi_sol)

# Plot the light ray path around the black hole
fig, ax = plt.subplots(figsize=(8, 8))
ax.plot(x_sol, y_sol, label="Light ray trajectory", color='orange')

# Draw the black hole event horizon
circle = plt.Circle((0, 0), a, color='black', alpha=0.7)
ax.add_patch(circle)

# Labels and visualization settings
ax.set_xlabel("x coordinate")
ax.set_ylabel("y coordinate")
ax.set_title("Light Ray Trajectory Around a Black Hole")
ax.legend()
ax.set_xlim(-24*a, 24*a)
ax.set_ylim(-24*a, 24*a)
ax.set_aspect('equal', adjustable='datalim')
ax.grid(True)

plt.show()
