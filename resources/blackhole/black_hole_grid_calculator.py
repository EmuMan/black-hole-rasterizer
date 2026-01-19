from scipy.integrate import solve_bvp
import numpy as np
import matplotlib.pyplot as plt

# Constants
a = 1  # Setting a normalized Schwarzschild radius for simplicity

# Define geodesic equations in first-order form for BVP solver
def geodesic_bvp(s: list[float], y: list[float]) -> list[float]:
    """
    System of geodesic equations rewritten for BVP.
    y = [t, r, phi, dt/ds, dr/ds, dphi/ds]
    """
    _t, r, _phi, dt, dr, dphi = y

    # Avoid division by zero near event horizon
    r = np.maximum(r, a + 1e-6)

    # Compute second derivatives
    d2t = - (a / r**2) * (1 - a / r)**-1 * dt * dr
    d2r = - (1/2) * (1 - a / r) * (a * dt**2 / r**2 - a * dr**2 / (r - a)**2 - 2 * r * dphi**2)
    d2phi = - (2 * dr * dphi) / r

    return np.array([dt, dr, dphi, d2t, d2r, d2phi])

def get_boundary_conditions(
        vertex_radial: tuple[float, float],
        observer_radial: tuple[float, float],
        phi_displacement: int
) -> list[float]:
    def _boundary_conditions(y0: list[float], y1: list[float]) -> list[float]:
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
        
        # 1. Enforce gauge: t(0) = 0
        bc_t_start = y0[0]  # should be 0

        # 2. Enforce starting spatial conditions:
        bc_r_start = y0[1] - vertex_radial[0]   # r(0) = r_start
        bc_phi_start = y0[2] - vertex_radial[1]   # φ(0) = φ_start

        # 3. Enforce ending spatial conditions:
        bc_r_end = y1[1] - observer_radial[0]     # r(1) = r_end
        bc_phi_end = y1[2] - observer_radial[1] - phi_displacement    # φ(1) = φ_end

        # 4. Enforce the null condition at s = 0:
        # Schwarzschild null geodesic condition:
        #   -(1 - a/r) * (dt/ds)^2 + (1 - a/r)^(-1) * (dr/ds)^2 + r^2 * (dφ/ds)^2 = 0.
        # We evaluate at s=0 (using y0 for dt/ds, dr/ds, and dφ/ds).
        metric_factor = 1 - a / y0[1]
        bc_null = - metric_factor * y0[3]**2 + (1 / metric_factor) * y0[4]**2 + y0[1]**2 * y0[5]**2

        # Return an array of all six boundary conditions.
        return np.array([bc_t_start, bc_r_start, bc_phi_start, bc_r_end, bc_phi_end, bc_null])
    
    return _boundary_conditions

def get_path(
        vertex_radial: tuple[float, float],
        observer_radial: tuple[float, float],
        resolution: int,
        wrap_negative: bool
) -> list[tuple[float, float]] | None:
    """
    Get the geodesic path from the vertex to the observer.

    Parameters:
    vertex_radial (tuple): (r, phi) - The radial coordinates of the vertex.
    observer_radial (tuple): (r, phi) - The radial coordinates of the observer.
    resolution (int): The number of points to sample along the path.
    wrap_negative (bool): Whether to wrap the other way around the black hole.

    Returns:
    list: [(x, y)] - The Cartesian coordinates of the geodesic path.
    """

    phi_displacement = 2 * np.pi if wrap_negative else 0
    
    # Initial guess for solution
    s_guess = np.linspace(0, 30, 300)  # Proper parameter range
    y_guess = np.zeros((6, len(s_guess)))
    y_guess[1] = np.linspace(vertex_radial[0], observer_radial[0], len(s_guess))  # r interpolation
    end_phi = observer_radial[1] + phi_displacement
    y_guess[2] = np.linspace(vertex_radial[1], end_phi, len(s_guess))  # phi interpolation

    # Solve the BVP
    boundary_conditions = get_boundary_conditions(vertex_radial, observer_radial, phi_displacement)
    solution = solve_bvp(geodesic_bvp, boundary_conditions, s_guess, y_guess)
    if not solution.success:
        # print(f"BVP solver failed to converge on {vertex_radial} to {observer_radial}.")
        return None

    # Extract the solution
    r = solution.y[1]
    phi = solution.y[2]

    # Convert to Cartesian coordinates
    x = r * np.cos(phi)
    y = r * np.sin(phi)

    return list(zip(x, y))

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

def get_total_length(points: list[tuple[float, float]]) -> float:
    """
    Compute the total length of a path defined by a list of points.

    Parameters:
    points (list): [(x, y)] - The Cartesian coordinates of the path.

    Returns:
    float: The total length of the path.
    """
    return np.sum(np.sqrt(np.diff(points, axis=0)[:, 0]**2 + np.diff(points, axis=0)[:, 1]**2))

def get_angle(p1: tuple[float, float], p2: tuple[float, float]) -> float:
    """
    Get the normalized direction vector from point p1 to point p2.

    Parameters:
    p1 (tuple): (x1, y1) - The first point.
    p2 (tuple): (x2, y2) - The second point.

    Returns:
    tuple: (dx, dy) - The normalized direction vector.
    """
    direction = (p2[0] - p1[0], p2[1] - p1[1])
    angle = np.arctan2(direction[1], direction[0])
    return angle

def save_info_to_file(
        vertex_angle_array: np.ndarray,
        observer_angle_array: np.ndarray,
        distance_array: np.ndarray,
        v_r_range: tuple[float, float],
        o_r_range: tuple[float, float],
        filename: str
):
    """
    Save a 3D NumPy array to a file.

    Parameters:
    array (np.ndarray): The 3D NumPy array to save.
    filename (str): The name of the file to save to.
    """
    assert vertex_angle_array.shape == observer_angle_array.shape == distance_array.shape
    shape = vertex_angle_array.shape
    with open(filename, 'w') as file:
        file.write(f"{shape[0]} {shape[1]} {shape[2]}\n")
        file.write(f"{v_r_range[0]} {v_r_range[1]}\n")
        file.write(f"{o_r_range[0]} {o_r_range[1]}\n")
        for i in range(shape[0]):
            for j in range(shape[1]):
                for k in range(shape[2]):
                    file.write(f"{vertex_angle_array[i, j, k]} {observer_angle_array[i, j, k]} {distance_array[i, j, k]}\n")

def interpolate_holes_in_array(array: np.ndarray) -> np.ndarray:
    """
    Interpolate holes in a 3D NumPy array.

    Parameters:
    array (np.ndarray): The 3D NumPy array with holes.

    Returns:
    np.ndarray: The 3D NumPy array with holes interpolated.
    """
    while True:
        holes = np.argwhere(np.isnan(array))
        if holes.size == 0:
            break
        print(f"Found {len(holes)} holes to interpolate.")
        for hole in holes:
            i, j, k = hole
            neighbors = []
            if i > 0:
                neighbor = array[i - 1, j, k]
                if not np.isnan(neighbor):
                    neighbors.append(neighbor)
            if i < array.shape[0] - 1:
                neighbor = array[i + 1, j, k]
                if not np.isnan(neighbor):
                    neighbors.append(neighbor)
            if j > 0:
                neighbor = array[i, j - 1, k]
                if not np.isnan(neighbor):
                    neighbors.append(neighbor)
            if j < array.shape[1] - 1:
                neighbor = array[i, j + 1, k]
                if not np.isnan(neighbor):
                    neighbors.append(neighbor)
            if k > 0:
                neighbor = array[i, j, k - 1]
                if not np.isnan(neighbor):
                    neighbors.append(neighbor)
            if k < array.shape[2] - 1:
                neighbor = array[i, j, k + 1]
                if not np.isnan(neighbor):
                    neighbors.append(neighbor)
            if neighbors:
                array[i, j, k] = np.mean(neighbors)
    return array

def plot_multiple(
        v_phi_resolution: int,
        v_r_resolution: int,
        v_r_max: float,
        o_r_resolution: int,
        o_r_max: float,
):
    v_r_min = 1.2
    o_r_min = 1.2

    v_phi = np.linspace(0, 2 * np.pi, v_phi_resolution)
    v_r = np.linspace(v_r_min, v_r_max, v_r_resolution)
    o_r = np.linspace(o_r_min, o_r_max, o_r_resolution)
    failure_count = 0

    shape = (v_r_resolution, v_phi_resolution, o_r_resolution)
    vertex_angles: np.ndarray = np.empty(shape) * np.nan
    observer_angles: np.ndarray = np.empty(shape) * np.nan
    distances: np.ndarray = np.empty(shape) * np.nan

    # fig, ax = plt.subplots(figsize=(8, 8))

    for v_r_i, r in enumerate(v_r):
        for v_phi_i, phi in enumerate(v_phi):
            vertex_radial = (r, phi)
            for o_r_i, o_r_val in enumerate(o_r):
                observer_radial = (o_r_val, 0)
                path = get_path(vertex_radial, observer_radial, 300, False)
                if not path:
                    failure_count += 1
                    continue
                vertex_angles[v_r_i][v_phi_i][o_r_i] = get_angle(path[0], path[1])
                observer_angles[v_r_i][v_phi_i][o_r_i] = get_angle(path[-1], path[-2])
                distances[v_r_i][v_phi_i][o_r_i] = get_total_length(path)
                # ax.plot(*zip(*path), color='blue', alpha=0.05)
    
    # ax.set_xlabel("x coordinate")
    # ax.set_ylabel("y coordinate")
    # ax.set_title("Geodesic Paths from Vertex to Observer (BVP)")
    # ax.set_xlim(-8*a, 8*a)
    # ax.set_ylim(-8*a, 8*a)
    # ax.set_aspect('equal', adjustable='datalim')
    # ax.grid(True)

    print(f"Failed to converge on {failure_count} paths. Ratio: {failure_count / (v_phi_resolution * v_r_resolution * o_r_resolution)}")

    interpolate_holes_in_array(vertex_angles)
    interpolate_holes_in_array(observer_angles)
    interpolate_holes_in_array(distances)
    save_info_to_file(
        vertex_angles,
        observer_angles,
        distances,
        (v_r_min, v_r_max),
        (o_r_min, o_r_max),
        f"blackhole_{v_phi_resolution}_{v_r_resolution}_{v_r_max}_{o_r_resolution}_{o_r_max}.txt"
    )

    # plt.show()

if __name__ == '__main__':
    # plot_multiple(128, 128, 32, 64, 32)
    plot_multiple(128, 64, 64, 64, 64)
