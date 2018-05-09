#include "MySolver.h"
using namespace Simplex;
//  MySolver
void MySolver::Init(void)
{
	m_v3Acceleration = ZERO_V3;
	m_v3Position = ZERO_V3;
	m_v3Velocity = ZERO_V3;
	m_fMass = 1.0f;
}
void MySolver::Swap(MySolver& other)
{
	std::swap(m_v3Acceleration, other.m_v3Acceleration);
	std::swap(m_v3Velocity, other.m_v3Velocity);
	std::swap(m_v3Position, other.m_v3Position);
	std::swap(m_fMass, other.m_fMass);
}
void MySolver::Release(void){/*nothing to deallocate*/ }
//The big 3
MySolver::MySolver(void){ Init(); }
MySolver::MySolver(MySolver const& other)
{
	m_v3Acceleration = other.m_v3Acceleration;
	m_v3Velocity = other.m_v3Velocity;
	m_v3Position = other.m_v3Position;
	m_fMass = other.m_fMass;
}
MySolver& MySolver::operator=(MySolver const& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MySolver temp(other);
		Swap(temp);
	}
	return *this;
}
MySolver::~MySolver() { Release(); }

//Accessors
void MySolver::SetPosition(vector3 a_v3Position) { m_v3Position = a_v3Position; }
vector3 MySolver::GetPosition(void) { return m_v3Position; }

void MySolver::SetVelocity(vector3 a_v3Velocity) { m_v3Velocity = a_v3Velocity; }
vector3 MySolver::GetVelocity(void) { return m_v3Velocity; }

void MySolver::SetMass(float a_fMass) { m_fMass = a_fMass; }
float MySolver::GetMass(void) { return m_fMass; }

//Methods
void MySolver::ApplyFriction(float a_fFriction)
{
	if (a_fFriction < 0.01f)
		a_fFriction = 0.01f;
	
	m_v3Velocity *= 1.0f - a_fFriction;

	//if velocity is really small make it zero
	if (glm::length(m_v3Velocity) < 0.01f)
		m_v3Velocity = ZERO_V3;
}
void MySolver::ApplyForce(vector3 a_v3Force)
{
	//check minimum mass
	if (m_fMass < 0.01f)
		m_fMass = 0.01f;
	//f = m * a -> a = f / m
	m_v3Acceleration += a_v3Force / m_fMass;
}
vector3 CalculateMaxVelocity(vector3 a_v3Velocity, float maxVelocity)
{
	if (glm::length(a_v3Velocity) > maxVelocity)
	{
		a_v3Velocity = glm::normalize(a_v3Velocity);
		a_v3Velocity *= maxVelocity;
	}
	return a_v3Velocity;
}
vector3 RoundSmallVelocity(vector3 a_v3Velocity, float minVelocity = 0.01f)
{
	if (glm::length(a_v3Velocity) < minVelocity)
	{
		a_v3Velocity = ZERO_V3;
	}
	return a_v3Velocity;
}
void MySolver::Update(void)
{
	ApplyForce(vector3(0.0f, -0.035f, 0.0f));

	m_v3Velocity += m_v3Acceleration;
	
	float fMaxVelocity = 5.0f;
	m_v3Velocity = CalculateMaxVelocity(m_v3Velocity, fMaxVelocity);

	ApplyFriction(0.1f);
	m_v3Velocity = RoundSmallVelocity(m_v3Velocity, 0.028f);

	m_v3Position += m_v3Velocity;
			
	if (!m_bSuckedIn) {
		if (m_v3Position.y <= m_fYPosition) {
			m_v3Position.y = m_fYPosition;
			m_v3Velocity.y = m_fYPosition;
		} else if (m_v3Position.y >= m_fYPosition) {
			m_v3Position.y = m_fYPosition;
			m_v3Velocity.y = m_fYPosition;
		}
	}
	else {
		if (m_v3Position.y <= m_fYPosition) {
			m_v3Position.y = m_fYPosition;
			m_v3Velocity.y = m_fYPosition;
		}
		else if (m_v3Position.y >= m_fYPosition + 0.8f) {
			m_v3Position.y = m_fYPosition + 0.8;
			m_v3Velocity.y = m_fYPosition + 0.8;
		}
	}
	

	m_v3Acceleration = ZERO_V3;
}
void MySolver::ResolveCollision(MySolver* a_pOther)
{
	float fMagThis = glm::length(m_v3Velocity);
	float fMagOther = glm::length(a_pOther->GetVelocity());

	if (IsGround() || a_pOther->IsGround()) return;

	if (IsBlackHole() && !a_pOther->IsBlackHole()) {
		a_pOther->SetIsDead();
		return;
	}
	else if (!IsBlackHole() && a_pOther->IsBlackHole()) {
		SetIsDead();
		return;
	} else if (IsBlackHole() && a_pOther->IsBlackHole()) {
		SetIsDead();
		a_pOther->SetIsDead();
		return;
	}

	m_v3Velocity.y = 0.0f;
	vector3 temp = a_pOther->GetVelocity();
	temp.y = 0.0f;

	if (m_v3Velocity == vector3() && temp == vector3()) {
		fMagThis = 0.0f;
		fMagOther = 0.0f;
	}

	if (fMagThis > 0.015f || fMagOther > 0.015f)
	{
		//a_pOther->ApplyForce(GetVelocity());

		ApplyForce(-a_pOther->GetVelocity());
		a_pOther->ApplyForce(m_v3Velocity);
	}
	else
	{
		m_v3Velocity.y = 1.0f;
		vector3 v3Direction = m_v3Position - a_pOther->GetPosition();
		if(glm::length(v3Direction) != 0)
			v3Direction = glm::normalize(v3Direction);
		v3Direction *= 0.3f;
		ApplyForce(v3Direction);
		a_pOther->ApplyForce(-v3Direction);
	}
}

void Simplex::MySolver::SetIsBlackHole()
{
	m_bIsBlackHole = true;
}

bool Simplex::MySolver::IsBlackHole()
{
	return m_bIsBlackHole;
}

bool Simplex::MySolver::IsGround()
{
	return m_bIsGround;
}

void Simplex::MySolver::SetIsGround()
{
	m_bIsGround = true;
}


void Simplex::MySolver::SetIsDead()
{
	m_bIsDead = true;
}

bool Simplex::MySolver::IsDead()
{
	return m_bIsDead;
}

void Simplex::MySolver::SetYPositionForGravityCheck(float a_fYPos)
{
	m_fYPosition = a_fYPos;
}

void Simplex::MySolver::BlackHolePull(vector3 a_v3Distance, uint a_nMass)
{
	a_v3Distance.y = 1.5f;
	float force = (0.03 * a_nMass * GetMass()) / glm::length(a_v3Distance);
	ApplyForce(glm::normalize(a_v3Distance) * force);
	m_bSuckedIn = true;
}
