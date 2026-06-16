#include <fstream>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>

/*
  matscan_voxel_prime.C

  This ROOT macro writes a Geant4 macro that approximates a voxelized
  material map by repeating /control/matScan measurements while shifting
  the eye position in 1 mm steps.

  Scan concept:
    - theta fixed to 0 deg
    - several phi angles (prime count)
    - z sampled from -10 to +10 cm (prime count)
    - eyePosition shifted in 1 mm steps along the fixed ray direction
    - material per 1 mm segment can be estimated by differencing successive
      cumulative measurements in post-processing.

  Output:
    - matscan_voxel_prime.mac
*/

void matscan_voxel_prime()
{
  const std::string outname = "matscan_voxel_prime.mac";
  std::ofstream out(outname);
  if (!out)
  {
    std::cerr << "Failed to open output file: " << outname << std::endl;
    return;
  }

  // Prime-number grids to reduce aliasing / beating with detector symmetries.
  const int Nz = 13;   // prime; about 1.01 mm spacing over 20 cm
  const int Nphi = 13;  // prime

  const double zmin_cm = -10.0;
  const double zmax_cm =  10.0;
  const double dz_cm = (zmax_cm - zmin_cm) / (Nz - 1);

  // Several phi angles over [0, 360) deg.
  const double phimin_deg = 0.0;
  const double phimax_deg = 360.0;
  const double dphi_deg = (phimax_deg - phimin_deg) / Nphi;

  // 1 mm increments along the fixed ray direction.
  const double step_cm = 0.2;
  const int nSteps = 250; // 20 cm path per ray

  out << std::fixed << std::setprecision(3);
  out << "# Auto-generated Geant4 macro for approximate voxelized material scan\n";
  out << "# theta fixed to 0 deg\n";
  out << "# z in [-10, 10] cm with prime-number sampling\n";
  out << "# phi in [0, 360) deg with prime-number sampling\n";
  out << "# eyePosition shifted in 1 mm steps along the ray\n\n";
  out << "/control/verbose 0\n";
  out << "/control/matScan/eyePosition 0 0 0 cm\n";
  out << "\n";

  for (int iz = 0; iz < Nz; ++iz)
  {
    const double z_cm = zmin_cm + iz * dz_cm;
    for (int iphi = 0; iphi < Nphi; ++iphi)
    {
      const double phi_deg = phimin_deg + iphi * dphi_deg;
      const double phi_rad = phi_deg * M_PI / 180.0;
      const double dx = std::cos(phi_rad);
      const double dy = std::sin(phi_rad);

      out << "# z = " << z_cm << " cm, phi = " << phi_deg << " deg\n";
      for (int istep = 0; istep < nSteps; ++istep)
      {
        const double s_cm = istep * step_cm;
        const double x_cm = s_cm * dx;
        const double y_cm = s_cm * dy;

        // Shift the eye position along the fixed ray.
        out << "/control/matScan/eyePosition "
            << x_cm << " " << y_cm << " " << z_cm << " cm\n";

        // Single measurement at theta = 0 deg, phi fixed.
        out << "/control/matScan/singleMeasure 0 " << phi_deg << " deg\n";
      }
      out << "\n";
    }
  }

  out.close();
  std::cout << "Wrote " << outname << std::endl;
}
