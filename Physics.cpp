#include "Physics.h"
PhysicObject::PhysicObject()
{
    shader = Shader("physObjShader.vert", "physObjShader.frag");
    width = 0.5;
    length = 0.5;
    height = 0.5;
    float quadVertices[] = {
         -0.5f, -0.5f,  0.5f,     0.0f,0.0f,1.0f,
          0.5f, -0.5f,  0.5f,     0.0f,0.0f,1.0f,
          0.5f,  0.5f,  0.5f,     0.0f,0.0f,1.0f,
         -0.5f, -0.5f,  0.5f,     0.0f,0.0f,1.0f,
          0.5f,  0.5f,  0.5f,     0.0f,0.0f,1.0f,
         -0.5f,  0.5f,  0.5f,     0.0f,0.0f,1.0f,

         -0.5f, -0.5f, -0.5f,     0.0f,0.0f,-1.0f,
         -0.5f,  0.5f, -0.5f,     0.0f,0.0f,-1.0f,
          0.5f,  0.5f, -0.5f,     0.0f,0.0f,-1.0f,
         -0.5f, -0.5f, -0.5f,     0.0f,0.0f,-1.0f,
          0.5f,  0.5f, -0.5f,     0.0f,0.0f,-1.0f,
          0.5f, -0.5f, -0.5f,     0.0f,0.0f,-1.0f,

          -0.5f, -0.5f, -0.5f,   -1.0f,0.0f,0.0f,
          -0.5f, -0.5f,  0.5f,   -1.0f,0.0f,0.0f,
          -0.5f,  0.5f,  0.5f,   -1.0f,0.0f,0.0f,
          -0.5f, -0.5f, -0.5f,   -1.0f,0.0f,0.0f,
          -0.5f,  0.5f,  0.5f,   -1.0f,0.0f,0.0f,
          -0.5f,  0.5f, -0.5f,   -1.0f,0.0f,0.0f,

           0.5f, -0.5f, -0.5f,    1.0f,0.0f,0.0f,
           0.5f,  0.5f, -0.5f,    1.0f,0.0f,0.0f,
           0.5f,  0.5f,  0.5f,    1.0f,0.0f,0.0f,
           0.5f, -0.5f, -0.5f,    1.0f,0.0f,0.0f,
           0.5f,  0.5f,  0.5f,    1.0f,0.0f,0.0f,
           0.5f, -0.5f,  0.5f,    1.0f,0.0f,0.0f,

          -0.5f,  0.5f, -0.5f,    0.0f,1.0f,0.0f,
          -0.5f,  0.5f,  0.5f,    0.0f,1.0f,0.0f,
           0.5f,  0.5f,  0.5f,    0.0f,1.0f,0.0f,
          -0.5f,  0.5f, -0.5f,    0.0f,1.0f,0.0f,
           0.5f,  0.5f,  0.5f,    0.0f,1.0f,0.0f,
           0.5f,  0.5f, -0.5f,    0.0f,1.0f,0.0f,

          -0.5f, -0.5f, -0.5f,    0.0f,-1.0f,0.0f,
           0.5f, -0.5f, -0.5f,    0.0f,-1.0f,0.0f,
           0.5f, -0.5f,  0.5f,    0.0f,-1.0f,0.0f,
          -0.5f, -0.5f, -0.5f,    0.0f,-1.0f,0.0f,
           0.5f, -0.5f,  0.5f,    0.0f,-1.0f,0.0f,
          -0.5f, -0.5f,  0.5f,    0.0f,-1.0f,0.0f
    };

    for (int i = 0; i < sizeof(quadVertices)/sizeof(quadVertices[0]); i += 6)
    {
        quadVertices[i] *= width;
        quadVertices[i + 1] *= height;
        quadVertices[i + 2] *= length;
    }

    for (int i = 0; i < 8; i++)
    {
        corners[i] = glm::vec3((i % 2 - 0.5) * width, (i / 2 % 2 - 0.5) * height, (i / 2 / 2 % 2 - 0.5) * length);
    }

    setInertiaTensor(glm::mat3((height*height+length*length) / 12.0, 0, 0, 0, (width*width+length*length) / 12.0, 0, 0, 0, (height*height+width*width) / 12.0));
    int normals[] =
    {
        4,
        5,
        3,
        2,
        0,
        1,
    };

    pos = glm::vec3(0.0);
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void PhysicObject::draw()
{
    shader.use();
    glBindVertexArray(quadVAO);
    shader.setVec3("offset", pos);
    shader.setVec3("cameraCol", calculateCameraCollision(Camera::getCamera()->pos, Camera::getCamera()->direction));
    shader.setMat3("rotation", rotationMat);
    shader.setVec3("cameraPos", Camera::getCamera()->pos);
    shader.setVec3("sunDirection", Camera::getCamera()->sunDirrection);
    shader.setVec3("sunColor", glm::vec3(1, 1, 1));
    GLint rotMatLoc = glGetUniformLocation(shader.ID, "rotMat");
    glUniformMatrix4fv(rotMatLoc, 1, GL_FALSE, glm::value_ptr(Camera::getCamera()->returnMatrixTransform()));
    glDrawArrays(GL_TRIANGLES,0,42);
} 

void PhysicObject::setInertiaTensor(glm::mat3 tensor)
{
    inverseInertiaTensor = glm::inverse(tensor);
}

void PhysicObject::transformInertiaTensor()
{
    inverseInertiaTensorWorld = rotationMat * inverseInertiaTensor * glm::transpose(rotationMat);
}

void PhysicObject::addForce(glm::vec3 force)
{
    forceAccum += force;
}

void PhysicObject::resetAccumulators()
{
    forceAccum = glm::vec3(0, 0, 0);
    torqueAccum = glm::vec3(0, 0, 0);
}

void PhysicObject::calculateDerivedData()
{
    transformInertiaTensor();
}

void PhysicObject::integrate(float deltaTime)
{
    rotationQuaternion = glm::normalize(rotationQuaternion);
    rotationMat = glm::mat3_cast(rotationQuaternion);
    glm::vec3 angularAcceleration =
        inverseInertiaTensorWorld*torqueAccum;
     velocity += forceAccum * inverseMass * deltaTime;
    rotationVelocity+=angularAcceleration*deltaTime;
    velocity *= glm::pow(linearDamping, deltaTime);
    rotationVelocity *= glm::pow(angularDamping, deltaTime);
    pos+=velocity* deltaTime;
    addScaledVector(rotationQuaternion, rotationVelocity, deltaTime);
    velocity *= glm::pow(linearDamping, deltaTime);
    rotationVelocity *= glm::pow(angularDamping, deltaTime);
    calculateDerivedData();
    resetAccumulators();
    rotationQuaternion = glm::normalize(rotationQuaternion);
    rotationMat = glm::mat3_cast(rotationQuaternion);
}

void PhysicObject::step(float deltaTime)
{
    for (ForceGenerator* c : myForces)
    {
        c->updateForce(this, deltaTime);
    }
    integrate(deltaTime);
}
void PhysicObject::addForceAtPoint(const glm::vec3& force, const glm::vec3& point)
{
    forceAccum += force;
    torqueAccum += glm::cross(point - pos, force);
    //torqueAccum += (point - pos) * force;
}

void PhysicObject::addForceAtBodyPoint(const glm::vec3& force, const glm::vec3& point)
{
    addForceAtPoint(force, point * rotationMat + pos);
}

glm::vec3 PhysicObject::calculateCameraCollision(glm::vec3 cameraPos, glm::vec3 cameraDirection)
{
    glm::vec3 normals[] = { glm::vec3(1,0,0),glm::vec3(-1,0,0),glm::vec3(0,1,0),glm::vec3(0,-1,0),glm::vec3(0,0,1),glm::vec3(0,0,-1) };
    glm::vec3 cameraPosRel = glm::inverse(rotationMat) * (cameraPos - pos);
    glm::vec3 cameraDirectionRel = glm::normalize(glm::inverse(rotationMat) * cameraDirection);
    float dist = -1;
    for (int i = 0; i < 6; i++)
    {
        float cdist = rayCollisionDist(cameraDirectionRel, cameraPosRel, normals[i], normals[i] * 0.5f * glm::vec3(width,height,length));
        if (!(cdist < 0) && (cdist < dist || dist == -1))
        {
            glm::vec3 tempPos = glm::abs(cameraDirectionRel * cdist + cameraPosRel);
            if(tempPos.x<=width/2.0&&tempPos.y<=height/2.0&&tempPos.z<=length/2.0)
                dist = cdist;
        }
    }
    if (dist == -1)
        return cameraDirection * -1.0f + cameraPos;
    return cameraDirectionRel * dist + cameraPosRel;
}

void PhysicObject::addForces(float time)
{
    //for (int i = 0; i < myForces.size(); i++)
    //    myForces[i].updateForce(this, time);
}

Box PhysicObject::toBox()
{
    return Box{ width,height,length,pos,rotationMat };
}
PhysicObject::operator Box() const
{
    return Box{ width,height,length,pos,rotationMat };
}
glm::vec3 PhysicObject::transformVector(glm::vec3 vector)
{
    return vector * rotationMat;
}

void rotateQuaternionByVector(glm::quat& quaternion, const glm::vec3& vector)
{
    glm::quat quat2(0, vector.x, vector.y, vector.z);
    quaternion = quaternion*quat2;
}

void addScaledVector(glm::quat& quat1, const glm::vec3& vector, float scale)
{
    glm::quat q = glm::quat(0, vector.x * scale, vector.y * scale, vector.z * scale);
    q = q * quat1;
    quat1.x += q.x * 0.5;
    quat1.y += q.y * 0.5;
    quat1.z += q.z * 0.5;
    quat1.w += q.w * 0.5;
}

glm::vec3 rayCollision(glm::vec3 rayDirection, glm::vec3 rayOrigin, glm::vec3 planeNormal, glm::vec3 pointOnPlane)
{
    return glm::dot(pointOnPlane - rayOrigin, planeNormal) / glm::dot(planeNormal, rayDirection) * rayDirection + rayOrigin;
}

float rayCollisionDist(glm::vec3 rayDirection, glm::vec3 rayOrigin, glm::vec3 planeNormal, glm::vec3 pointOnPlane)
{
    return glm::dot(pointOnPlane - rayOrigin, planeNormal) / glm::dot(planeNormal, rayDirection);
}

void ForceGenerator::updateForce(PhysicObject* myObject, float time) {
}

void Gravity::updateForce(PhysicObject* myObject, float time)
{
    myObject->addForce(gravity / (myObject->inverseMass));
}

void Spring::updateForce(PhysicObject* myObject, float time)
{
    glm::vec3 globalCP = myObject->rotationMat * connectionPoint + myObject->pos;
    glm::vec3 connectionVector = globalCP - springPos;
    float mag = glm::length(connectionVector);
    mag = abs(mag - springDis);
    mag *= springStrength;
    connectionVector = glm::normalize(connectionVector);
    connectionVector *= -mag;
    myObject->addForceAtBodyPoint(connectionVector, connectionPoint);
}

void boxAndPointCollision(const Box& myObject, const glm::vec3& myPoint, CollisionData& data)
{
    glm::vec3 relativePoint = glm::inverse(myObject.rotation) * (myPoint - myObject.pos);
    glm::vec3 normal;
    float min_depth = myObject.width/2.0 - glm::abs(relativePoint.x);
    if (min_depth < 0) return;
    normal = glm::vec3(((relativePoint.x < 0) ? -1 : 1),0,0);
    float depth = myObject.height/2.0 - glm::abs(relativePoint.y);
    if (depth < 0) return;
    else if (depth < min_depth)
    {
        min_depth = depth;
        normal = glm::vec3(0, ((relativePoint.y < 0) ? -1 : 1), 0);
    }
    depth = myObject.length/2.0 - glm::abs(relativePoint.z);
    if (depth < 0) return;
    else if (depth < min_depth)
    {
        min_depth = depth;
        normal = normal = glm::vec3(0, 0, ((relativePoint.x < 0) ? -1 : 1));
    }
    Contact myContact;
    myContact.contactNormal = normal;
    myContact.contactPoint = myPoint;
    myContact.depth = min_depth;
    data.contacts.push_back(myContact);
}
void boxAndPointCollision(const PhysicObject& myObject, const glm::vec3& myPoint, CollisionData& data)
{
    if (glm::all(glm::greaterThan(glm::abs(myObject.pos - myPoint), glm::vec3(myObject.width / 2.0f, myObject.height / 2.0f, myObject.length / 2.0f))))
        return;
    glm::vec3 relativePoint = glm::inverse(myObject.rotationMat) * (myPoint - myObject.pos);
    glm::vec3 normal;
    float min_depth = myObject.width / 2.0 - glm::abs(relativePoint.x);
    if (min_depth < 0) return;
    normal = glm::vec3(((relativePoint.x < 0) ? -1 : 1), 0, 0);
    float depth = myObject.height / 2.0 - glm::abs(relativePoint.y);
    if (depth < 0) return;
    else if (depth < min_depth)
    {
        min_depth = depth;
        normal = glm::vec3(0, ((relativePoint.y < 0) ? -1 : 1), 0);
    }
    depth = myObject.length / 2.0 - glm::abs(relativePoint.z);
    if (depth < 0) return;
    else if (depth < min_depth)
    {
        min_depth = depth;
        normal = normal = glm::vec3(0, 0, ((relativePoint.x < 0) ? -1 : 1));
    }
    Contact myContact;
    myContact.contactNormal = normal;
    myContact.contactPoint = myPoint;
    myContact.depth = min_depth;
    data.contacts.push_back(myContact);
}
bool boxAndPointCollision(const Box& myObject, const glm::vec3& myPoint)
{
    glm::vec3 relativePoint = glm::inverse(myObject.rotation) * (myPoint - myObject.pos);
    if (glm::all(glm::greaterThan(glm::abs(relativePoint), glm::vec3(myObject.width / 2.0f, myObject.height / 2.0f, myObject.length / 2.0f))))
        return false;
    return true;
}
void boxAndBoxCollision(const Box& box1, const Box& box2, CollisionData& data)
{
    for (int i = 0; i < 8; i++)
    {
        boxAndPointCollision(box1, returnPoint(box2,i),data);
        boxAndPointCollision(box2, returnPoint(box1, i), data);
    }
}
void boxAndObjectCollision(const Box& box1, const Box& object, CollisionData& data)
{

    for (int i = 0; i < 8; i++)
    {
        boxAndPointCollision(box1, returnPoint(object,i), data);
    }
}
glm::vec3 returnPoint(const Box& box, const int corner)
{
    return box.rotation * glm::vec3(box.width * ((corner % 8) % 2 - 0.5f), box.height * ((corner % 8) / 2 % 2 - 0.5f), box.length * ((corner % 8) / 4 - 0.5f)) + box.pos;
}
glm::vec3 closestPoint(glm::vec3 p1, glm::vec3 q1, glm::vec3 p2, glm::vec3 q2) {
    glm::vec3 d1 = q1 - p1;
    glm::vec3 d2 = q2 - p2;
    glm::vec3 r = p1 - p2;
    float a = glm::dot(d1, d1);
    float e = glm::dot(d2, d2);
    float f = glm::dot(d2, r);

    float s, t;
    float c = glm::dot(d1, r);
    float b = glm::dot(d1, d2);
    float denom = a * e - b * b;

    if (denom != 0.0f) {
        s = glm::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
    }
    else {
        s = 0.0f;
    }

    t = (b * s + f) / e;
    if (t < 0.0f) {
        t = 0.0f;
        s = glm::clamp(-c / a, 0.0f, 1.0f);
    }
    else if (t > 1.0f) {
        t = 1.0f;
        s = glm::clamp((b - c) / a, 0.0f, 1.0f);
    }

    return p1 + d1 * s;
}

void edgeAndEdgeCollision(const Box& box1, const Box& box2, CollisionData& data)
{
    Contact contact;
    contact.depth = -1;
    for (int i = 0; i < 12; i++)
    {
        edge edge1 = getEdge(box2,i);
        for (int j = 0; j < 12; j++)
        {
            edge edge2 = getEdge(box1, j);
            glm::vec3 point1 = closestPoint(edge1.point1, edge1.point2, edge2.point1, edge2.point2);
            glm::vec3 point2 = closestPoint(edge2.point1, edge2.point2, edge1.point1, edge1.point2);
            float depth = glm::distance(point1, point2);
            if ((depth < contact.depth || contact.depth == -1) && boxAndPointCollision(box1, point1))
            {
                contact.depth = depth;
                contact.contactPoint = point1;
                contact.contactNormal = glm::normalize(point2 - point1);
            }
        }
    }
    if (contact.depth != -1)
    {
        data.contacts.push_back(contact);
    }
}

edge getEdge(const Box& box, int number)
{
    glm::vec3 point1;
    glm::vec3 point2;

    if (number < 4)
    {
        point1 = glm::vec3(-box.width / 2.0f, (number % 2 - 0.5f) * box.height, (number / 2 % 2 - 0.5f) * box.length);
        point2 = glm::vec3(box.width / 2.0f, (number % 2 - 0.5f) * box.height, (number / 2 % 2 - 0.5f) * box.length);
    }
    else if (number < 8)
    {
        point1 = glm::vec3((number % 2 - 0.5f) * box.width, -box.height / 2.0f, (number / 2 % 2 - 0.5f) * box.length);
        point2 = glm::vec3((number % 2 - 0.5f) * box.width, box.height / 2.0f, (number / 2 % 2 - 0.5f) * box.length);
    }
    else
    {
        point1 = glm::vec3((number % 2 - 0.5f) * box.width, (number / 2 % 2 - 0.5f) * box.height, -box.length / 2.0f);
        point2 = glm::vec3((number % 2 - 0.5f) * box.width, (number / 2 % 2 - 0.5f) * box.height, box.length / 2.0f);
    }
    point1 = box.rotation * point1;
    point2 = box.rotation * point2;
    point1 += box.pos;
    point2 += box.pos;
    return edge{ point1,point2 };
}

void makeOrthonormalBasis(const glm::vec3& x, glm::vec3* y, glm::vec3* z)
{
    *z = glm::cross(x,*y);
    if (z->length() * z->length() == 0.0) return;
    (*y) = glm::cross((*z), x);
    *y = glm::normalize(*y);
    *z = glm::normalize(*z);
}