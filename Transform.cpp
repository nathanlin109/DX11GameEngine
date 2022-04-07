#include "Transform.h"

using namespace DirectX;

Transform::Transform()
{
    // Set up our initial transform values
    SetPosition(0, 0, 0);
    SetRotation(0, 0, 0);
    SetScale(1, 1, 1);

    // Create our initial matrix
    XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
    XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
    matrixDirty = false;
}

void Transform::MoveAbsolute(float x, float y, float z)
{
    // One way to do this is to just add the floats
    position.x += x;
    position.y += y;
    position.z += z;

    // Alternatively, we could use the math library
    //XMVECTOR pos = XMLoadFloat3(&position);
    //XMVECTOR offset = XMVectorSet(x, y, z, 0);
    //XMStoreFloat3(&position, pos + offset);

    matrixDirty = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
    // My initial movement vector
    XMVECTOR moveVec = XMVectorSet(x, y, z, 0);

    // Rotate the movement vector by this transform's orientation
    XMVECTOR rotatedVec = XMVector3Rotate(
        moveVec,
        XMQuaternionRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z));

    // Add the rotated movement vector to my
    // position and overwrite the old position
    XMVECTOR newPos = XMLoadFloat3(&position) + rotatedVec;
    XMStoreFloat3(&position, newPos);

    // Remember that the matrices are out of date
    matrixDirty = true;
}

void Transform::Rotate(float p, float y, float r)
{
    pitchYawRoll.x += p;
    pitchYawRoll.y += y;
    pitchYawRoll.z += r;
    matrixDirty = true;
}

void Transform::Scale(float x, float y, float z)
{
    scale.x *= x;
    scale.y *= y;
    scale.z *= z;
    matrixDirty = true;
}

void Transform::SetPosition(float x, float y, float z)
{
    position.x = x;
    position.y = y;
    position.z = z;
    matrixDirty = true;
}

void Transform::SetRotation(float p, float y, float r)
{
    pitchYawRoll.x = p;
    pitchYawRoll.y = y;
    pitchYawRoll.z = r;
    matrixDirty = true;

    // Updates the forward, up, and right vectors
    // Overrites the forward vector
    XMStoreFloat3(&forward, XMVector3Rotate(
        XMVectorSet(0, 0, 1, 0),
        XMQuaternionRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z)));

    // Overrites the up vector
    XMStoreFloat3(&up, XMVector3Rotate(
        XMVectorSet(0, 1, 0, 0),
        XMQuaternionRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z)));

    // Crosses the forward and up vectors to get the right vector
    XMStoreFloat3(&right, XMVector3Cross(XMLoadFloat3(&forward),
        XMLoadFloat3(&up)));
}

void Transform::SetScale(float x, float y, float z)
{
    scale.x = x;
    scale.y = y;
    scale.z = z;
    matrixDirty = true;
}

XMFLOAT3 Transform::GetPosition(){ return position; }

XMFLOAT3 Transform::GetPitchYawRoll(){ return pitchYawRoll; }

XMFLOAT3 Transform::GetScale(){ return scale; }

XMFLOAT3 Transform::GetForward(){ return forward; }

XMFLOAT3 Transform::GetUp(){ return up; }

XMFLOAT3 Transform::GetRight(){ return right; }

XMFLOAT4X4 Transform::GetWorldMatrix()
{
    UpdateMatrices();
    return worldMatrix;
}

XMFLOAT4X4 Transform::GetworldInverseTransposeMatrix()
{
    UpdateMatrices();
    return worldInverseTransposeMatrix;
}

void Transform::UpdateMatrices() {
    // Only do the heavy lifting if the matrix is out of date
    if (matrixDirty)
    {
        // Create the individual transformation matrices
        // for each type of transform
        XMMATRIX transMat = XMMatrixTranslation(position.x, position.y, position.z);
        XMMATRIX rotMat = XMMatrixRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z);
        XMMATRIX scaleMat = XMMatrixScaling(scale.x, scale.y, scale.z);

        // Combine them and store the result
        XMMATRIX worldMat = scaleMat * rotMat * transMat;
        XMStoreFloat4x4(&worldMatrix, worldMat);
        XMStoreFloat4x4(&worldInverseTransposeMatrix,
            XMMatrixInverse(0, XMMatrixTranspose(worldMat)));

        // Remember that we're clean
        matrixDirty = false;
    }
}