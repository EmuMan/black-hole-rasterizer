from scipy.integrate import solve_bvp
import numpy as np
import matplotlib.pyplot as plt

# Constants
a = 1  # Setting a normalized Schwarzschild radius for simplicity
vertex = (10 * a, np.pi - 0.6)
observer = (5 * a, 0)

# Define geodesic equations in first-order form for BVP solver
def geodesic_bvp(s, y):
    """
    System of geodesic equations rewritten for BVP.
    y = [t, r, phi, dt/ds, dr/ds, dphi/ds]
    """
    t, r, phi, dt, dr, dphi = y

    # Avoid division by zero near event horizon
    r = np.maximum(r, a + 1e-6)

    # Compute second derivatives
    d2t = - (a / r**2) * (1 - a / r)**-1 * dt * dr
    d2r = - (1/2) * (1 - a / r) * (a * dt**2 / r**2 - a * dr**2 / (r - a)**2 - 2 * r * dphi**2)
    d2phi = - (2 * dr * dphi) / r

    return np.array([dt, dr, dphi, d2t, d2r, d2phi])

phi_displacement = 0

def boundary_conditions(y0, y1):
    """
    Boundary conditions for the geodesic BVP:
      - t(0) = 0 (gauge condition)
      - r(0) = r_start and φ(0) = φ_start (initial spatial point)
      - r(1) = r_end and φ(1) = φ_end (final spatial point)
      - Null condition enforced at s = 0.
    
    Here, y = [t, r, φ, dt/ds, dr/ds, dφ/ds].
    """
    # Use the predefined vertex and observer as starting and ending positions.
    # vertex = (r_start, φ_start)
    # observer = (r_end, φ_end)

    global phi_displacement
    
    # 1. Enforce gauge: t(0) = 0
    bc_t_start = y0[0]  # should be 0

    # 2. Enforce starting spatial conditions:
    bc_r_start = y0[1] - vertex[0]   # r(0) = r_start
    bc_phi_start = y0[2] - vertex[1]   # φ(0) = φ_start

    # 3. Enforce ending spatial conditions:
    bc_r_end = y1[1] - observer[0]     # r(1) = r_end
    bc_phi_end = y1[2] - observer[1] - phi_displacement    # φ(1) = φ_end

    # 4. Enforce the null condition at s = 0:
    # Schwarzschild null geodesic condition:
    #   -(1 - a/r) * (dt/ds)^2 + (1 - a/r)^(-1) * (dr/ds)^2 + r^2 * (dφ/ds)^2 = 0.
    # We evaluate at s=0 (using y0 for dt/ds, dr/ds, and dφ/ds).
    metric_factor = 1 - a / y0[1]
    bc_null = - metric_factor * y0[3]**2 + (1 / metric_factor) * y0[4]**2 + y0[1]**2 * y0[5]**2

    # Return an array of all six boundary conditions.
    return np.array([bc_t_start, bc_r_start, bc_phi_start, bc_r_end, bc_phi_end, bc_null])

def to_cartesian(radial: tuple[float, float]) -> tuple[float, float]:
    """
    Convert from radial coordinates to Cartesian coordinates.

    Parameters:
    radial (tuple): (r, phi) - The radial coordinates.

    Returns:
    tuple: (x, y) - The Cartesian coordinates.
    """
    r, phi = radial
    x = r * np.cos(phi)
    y = r * np.sin(phi)
    return x, y

# Initial guess for solution
s_guess = np.linspace(0, 30, 300)  # Proper parameter range

# Guess initial trajectory for primary ray (straight-line approximation)
y_guess_primary = np.zeros((6, len(s_guess)))
y_guess_primary[1] = np.linspace(vertex[0], observer[0], len(s_guess))  # r interpolation
y_guess_primary[2] = np.linspace(vertex[1], observer[1], len(s_guess))  # phi interpolation

# Guess initial trajectory for secondary ray around the black hole (opposite to primary)
y_guess_secondary = np.zeros((6, len(s_guess)))
y_guess_secondary[1] = np.linspace(vertex[0], observer[0], len(s_guess))  # r interpolation
y_guess_secondary[2] = np.linspace(vertex[1], observer[1] + 2 * np.pi, len(s_guess))  # phi interpolation

# Solve the BVP
solution_primary = solve_bvp(geodesic_bvp, boundary_conditions, s_guess, y_guess_primary)
phi_displacement = 2 * np.pi
solution_secondary = solve_bvp(geodesic_bvp, boundary_conditions, s_guess, y_guess_secondary)

# Extract the solution
r_primary = solution_primary.y[1]
phi_primary = solution_primary.y[2]

r_secondary = solution_secondary.y[1]
phi_secondary = solution_secondary.y[2]

# r_secondary = y_guess_secondary[1]
# phi_secondary = y_guess_secondary[2]

# Convert to Cartesian coordinates
x_primary = r_primary * np.cos(phi_primary)
y_primary = r_primary * np.sin(phi_primary)

x_secondary = r_secondary * np.cos(phi_secondary)
y_secondary = r_secondary * np.sin(phi_secondary)

direction_hit_observer_primary = (x_primary[-2] - observer[0], y_primary[-2] - observer[1])
direction_hit_observer_primary = direction_hit_observer_primary / np.linalg.norm(direction_hit_observer_primary)
print(f"Primary ray direction at observer: {direction_hit_observer_primary}")

direction_hit_observer_secondary = (x_secondary[-2] - observer[0], y_secondary[-2] - observer[1])
direction_hit_observer_secondary = direction_hit_observer_secondary / np.linalg.norm(direction_hit_observer_secondary)
print(f"Secondary ray direction at observer: {direction_hit_observer_secondary}")

total_length_primary = np.sum(np.sqrt(np.diff(x_primary)**2 + np.diff(y_primary)**2))
total_length_secondary = np.sum(np.sqrt(np.diff(x_secondary)**2 + np.diff(y_secondary)**2))
print(f"Primary ray length: {total_length_primary}")
print(f"Secondary ray length: {total_length_secondary}")

# Plot the result
fig, ax = plt.subplots(figsize=(8, 8))
ax.plot(x_primary, y_primary, label="Primary Solution (BVP)", color='orange')
ax.plot(x_secondary, y_secondary, label="Secondary Solution (BVP)", color='green')

# Draw the black hole event horizon
circle = plt.Circle((0, 0), a, color='black', alpha=0.7)
ax.add_patch(circle)

# Mark observer and vertex
ax.scatter(*to_cartesian(vertex), color='blue', s=100, label="Vertex")
ax.scatter(*to_cartesian(observer), color='red', s=100, label="Observer")

# Labels and visualization settings
ax.set_xlabel("x coordinate")
ax.set_ylabel("y coordinate")
ax.set_title("Geodesic Path from Vertex to Observer (BVP)")
ax.legend()
ax.set_xlim(-12*a, 12*a)
ax.set_ylim(-12*a, 12*a)
ax.set_aspect('equal', adjustable='datalim')
ax.grid(True)

plt.show()