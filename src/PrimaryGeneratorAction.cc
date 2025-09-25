#include "PrimaryGeneratorAction.hh"

#include "SimConfig.hh"
#include "G4IonTable.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4PhysicalConstants.hh"

namespace Test {

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  fParticleGun  = new G4ParticleGun(1);
  auto* gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
  fParticleGun->SetParticleDefinition(gamma);
  fParticleGun->SetParticleEnergy(10.*keV);
  fParticleGun->SetParticleMomentumDirection({0,0,1});
  fParticleGun->SetParticlePosition({0,0,0});
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() { delete fParticleGun; }

G4ThreeVector PrimaryGeneratorAction::SamplePosInLXe(const SimConfig::Geometry& g) {
  const G4double R = g.lxe_radius, H = g.lxe_height;
  G4double u = G4UniformRand();
  G4double r = R*std::sqrt(u);
  G4double phi = 2.*pi*G4UniformRand();
  G4double x = r*std::cos(phi), y = r*std::sin(phi);
  G4double z = (G4UniformRand()-0.5)*H;
  return {x,y,z};
}

void PrimaryGeneratorAction::GenerateCategory(EventCategory cat, G4Event* evt) {
  auto& cfg = SimConfig::Get();
  auto& g   = cfg.geom;
  auto& gen = cfg.gen;

  auto* ion = G4IonTable::GetIonTable()->GetIon(gen.ion_Z, gen.ion_A, 0.*keV);

  auto shoot = [&](const G4ThreeVector& pos, G4double t_ns){
    G4double c = 2.*G4UniformRand()-1.;
    G4double s = std::sqrt(1.-c*c);
    G4double ph = 2.*pi*G4UniformRand();
    G4ThreeVector dir{s*std::cos(ph), s*std::sin(ph), c};

    G4double Enr = gen.E_min + (gen.E_max - gen.E_min)*G4UniformRand();

    fParticleGun->SetParticleDefinition(ion);
    fParticleGun->SetParticleCharge(0.*eplus);
    fParticleGun->SetParticlePosition(pos);
    fParticleGun->SetParticleMomentumDirection(dir);
    fParticleGun->SetParticleEnergy(Enr);
    fParticleGun->SetParticleTime(t_ns*ns);
    fParticleGun->GeneratePrimaryVertex(evt);
  };

  if (cat == EventCategory::Single) {
    shoot(SamplePosInLXe(g), 0.0);
  } else if (cat == EventCategory::DoubleNear) {
    auto p0 = SamplePosInLXe(g);
    G4ThreeVector dp(gen.near_dr, 0, 0);
    shoot(p0, 0.0);
    shoot(p0 + dp, gen.near_dt_ns);
  } else if (cat == EventCategory::DoubleFar) {
    auto p0 = SamplePosInLXe(g);
    auto p1 = SamplePosInLXe(g);
    while ((p1 - p0).mag() < gen.far_min_dr) p1 = SamplePosInLXe(g);
    shoot(p0, 0.0);
    shoot(p1, gen.far_dt_ns);
  } else if (cat == EventCategory::Triple) {
    shoot(SamplePosInLXe(g), gen.triple_t_ns_1);
    shoot(SamplePosInLXe(g), gen.triple_t_ns_2);
    shoot(SamplePosInLXe(g), gen.triple_t_ns_3);
  }
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  auto& cfg = SimConfig::Get();
  if (cfg.gen.use_wimp_proxy) {
    auto cat = static_cast<EventCategory>(cfg.gen.event_category);
    GenerateCategory(cat, anEvent);
  } else {
    auto& gen = cfg.gen;
    auto* gamma = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
    fParticleGun->SetParticleDefinition(gamma);
    fParticleGun->SetParticleEnergy(gen.gamma_energy);
    fParticleGun->SetParticlePosition(gen.gamma_position);
    fParticleGun->SetParticleMomentumDirection(gen.gamma_direction);
    fParticleGun->GeneratePrimaryVertex(anEvent);
  }
}

} // namespace Test
