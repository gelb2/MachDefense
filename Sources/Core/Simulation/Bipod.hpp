#pragma once
#include "Core/Simulation/Battle.hpp"
#include <array>
#include <span>

namespace md {
struct BipodFootPose { float angle=0,addPosX=0,scale=1; };
// GobHvMach::processMove_Bipod (0x11fcc). Coordinates and angles remain
// original world coordinates; the renderer converts them to screen coordinates.
struct BipodMotion {
    Vec2 position,destination,direction;
    float angle=0,speed=0;
    float bodyAngle=0,baseAngle=0,leftWeaponAngle=0,rightWeaponAngle=0;
    float bodyScale=1,bodyAddPosY=0;
    std::array<BipodFootPose,2> feet{}; // left, right
    bool weaponReady=false;
    // Original moveInfo byte 0 bit 0. True requests the phase-4 planted turn.
    bool plantedTurn=false,rightFootActive=false;
    int state=0,phase=0,speedPoint=0;
    float stepSize=10,stepPosition=0,bodyAdvance=0;
    void setDestination(Vec2 point);
    void setStepSize(float value);
    void footChange();
    void tick(std::optional<Vec2> target=std::nullopt,
              std::optional<CollisionSurface> collision=std::nullopt);
};
struct AlliedObstacle { Vec2 position;float radius=0;int hp=0; };
// Pass other allies only, excluding this unit. 0xe8e8 checks each live ally.
Vec2 checkPlayerMoveAhead(Vec2 position,Vec2 requested,float radius,float deviceScale,
                          std::span<const AlliedObstacle> others);
struct PlayerMovementFrame {
    double time=0;
    Vec2 position,destination;
    float targetRange=0,baseBoundary=0,deviceScale=.5f,radius=0;
    bool hasTarget=false,moveToTarget=false;
};
// GobHvM_Player::aiMove (0xee38). The caller applies returned destinations and
// copies topPlayerPosition back to GobWorld's equivalent state.
struct PlayerSteering {
    float stopDelay=0,topPlayerPosition=0;
    bool isOrder=false;
    std::optional<Vec2> tick(const PlayerMovementFrame&,DarwinRandom&,
                            std::span<const AlliedObstacle> others={});
};
}
