/*
Tao Du
taodu@stanford.edu
Jun 4, 2014
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PBRT_VOLUMES_AURORA_H
#define PBRT_VOLUMES_AURORA_H

// volumes/aurora.h*
#include "volume.h"
#include "../core/splines.h"

#include <fstream>

struct AuroraPhoton
{
	AuroraPhoton(const Point& pp, float rr, float gg, float bb)
		: p(pp), r(rr), g(gg), b(bb)
	{
	}
	Point p;
	float r, g, b;
};

struct AuroraVoxel
{
	AuroraVoxel()
	{
		photons.clear();
	}
	~AuroraVoxel()
	{
		photons.clear();
	}
	int PhotonNum() {return (int)photons.size();}
	vector<AuroraPhoton> photons;
};

class AuroraGrid
{
public:
	AuroraGrid(const BBox &e, int x, int y, int z, float radius,
		const string rcolor, const string gcolor, const string bcolor,
		const string intensity)
	{
		Vector vox = e.pMax - e.pMin;
		float dx = vox.x / x;
		float dy = vox.y / y;
		float dz = vox.z / z;
		step = max(dx, max(dy, dz));
		step = max(step, 2 * radius);
		nx = int(vox.x / step) + 1;
		ny = int(vox.y / step) + 1;
		nz = int(vox.z / step) + 1;
		vox.x = step * nx;
		vox.y = step * ny;
		vox.z = step * nz;
		extent.pMin = e.pMin;
		extent.pMax = e.pMin + vox;
		grids = new AuroraVoxel[nx * ny * nz];

		//	color and intensity
		//	read aurora color information
		std::ifstream fin;
		float height, value;
		//	r color
		fin.open(rcolor);
		while (!fin.eof())
		{
			fin >> height >> value;
			auroraColor[0].Add_Sample(height, value);
		}
		fin.close();
		//	g color
		fin.open(gcolor);
		while (!fin.eof())
		{
			fin >> height >> value;
			auroraColor[1].Add_Sample(height, value);
		}
		fin.close();
		//	b color
		fin.open(bcolor);
		while (!fin.eof())
		{
			fin >> height >> value;
			auroraColor[2].Add_Sample(height, value);
		}
		fin.close();
		//	intensity
		fin.open(intensity);
		while (!fin.eof())
		{
			fin >> height >> value;
			auroraIntensity.Add_Sample(height, value);
		}
		fin.close();
	}
	~AuroraGrid(){delete []grids;}

	//	add a new photon into the grid
	void AddPhoton(const AuroraPhoton &photon);
	//	given a position p and a search radius, find the nearby photon
	//	then use gaussian kernel to average them
	void SearchInGrid(const Point &p, float &r, float &g, float &b) const;

private:
	BBox extent;
	float step;
	float radius;
	int nx, ny, nz;
	AuroraVoxel *grids;

	Catmull_Rom auroraColor[3];
	Catmull_Rom auroraIntensity;
};


// AuroraDensity Declarations
class AuroraDensity : public VolumeRegion {
public:
	// AuroraRegion Public Methods
    AuroraDensity(const Spectrum &sa, const Spectrum &ss, float gg, const BBox &e,
                  const Transform &VolumeToWorld, float aa, float bb,
                       const Vector &up, int x, int y, int z, float radius, const float *d,
					   const string rcolor, const string gcolor, const string bcolor, const string intensity)
        : sig_a(sa), sig_s(ss), g(gg), extent(e),
          WorldToVolume(Inverse(VolumeToWorld)), a(aa), b(bb), 
		  nx(x), ny(y), nz(z), grid(e, nx, ny, nz, radius, rcolor, gcolor, bcolor, intensity)
	{
		upDir = Normalize(up);
		eleDensity = new float[nx*ny*nz];
        memcpy(eleDensity, d, nx * ny * nz * sizeof(float));
		//	TODO
	}
	~AuroraDensity();
	BBox WorldBound() const { return Inverse(WorldToVolume)(extent); }
	bool IntersectP(const Ray &r, float *t0, float *t1) const 
	{
        Ray ray = WorldToVolume(r);
        return extent.IntersectP(ray, t0, t1);
    }
	float Density(const Point &Pobj) const;
    Spectrum sigma_a(const Point &p, const Vector &, float) const 
	{
        return Density(WorldToVolume(p)) * sig_a;
    }
    Spectrum sigma_s(const Point &p, const Vector &, float) const 
	{
        return Density(WorldToVolume(p)) * sig_s;
    }
    Spectrum sigma_t(const Point &p, const Vector &, float) const 
	{
        return Density(WorldToVolume(p)) * (sig_a + sig_s);
    }
    Spectrum Lve(const Point &p, const Vector &, float) const;	//	TODO
    float p(const Point &p, const Vector &w, const Vector &wp, float) const 
	{
        return PhaseHG(w, wp, g);
    }
    Spectrum tau(const Ray &r, float stepSize, float offset) const;

	float EleDensity(const Point &Pobj) const;

	float D(int x, int y, int z) const {
        x = Clamp(x, 0, nx-1);
        y = Clamp(y, 0, ny-1);
        z = Clamp(z, 0, nz-1);
        return eleDensity[z*nx*ny + y*nx + x];
    }
private:
	Spectrum sig_a, sig_s;
    float g;
    Transform WorldToVolume;
	BBox extent;
	
	//	use a, b and upDir to compute density
	float a, b;
    Vector upDir;

	//	eleDensity represents whether aurora 'exists' or not
	//	use eleDensity to generate the photon in aurora
	float *eleDensity;
	//	the number of bins in x, y, z in eDensity
	int nx, ny, nz;

	//	aurora grid
	AuroraGrid grid;
};


AuroraDensity *CreateAuroraVolumeRegion(const Transform &volume2world,
        const ParamSet &params);

#endif // PBRT_VOLUMES_AURORA_H
