//
// Created by dusica on 25.8.22..
//

#ifndef PROJECT_BASE_CUBE_H
#define PROJECT_BASE_CUBE_H


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ctime>
#include <cstdlib>
#include<chrono>
#include <iostream>
#include <random>

float RandomPosition()
{
    float positions[] = {-0.5, -0.25, 0.0, 0.25, 0.5};
    std::default_random_engine generator( std::random_device{}() );
    std::uniform_int_distribution<int> distribution(0,4);
    int index = distribution(generator);
    std::cerr << index << std::endl;
    return positions[index];

}

class Cube {
public:

    Cube(bool isPoint) {
        float x = RandomPosition();
        m_xCoord = x;
        m_isPoint = isPoint;

    }

    Cube(float x, bool isPoint) {
        m_xCoord = x;
        m_isPoint = isPoint;

    }

    float getXCoord() const {
        return m_xCoord;
    }
    float getZCoord() const {
        return m_zCoord;
    }

    float getYCoord() const {
        return m_yCoord;
    }

    glm::mat4 translateCube(float x,float y, float z) {

        m_xCoord = x;
        m_yCoord = y;
        m_zCoord = z;

        float scaleFactor = isPoint() ? 0.1f: 0.3f;
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model,glm::vec3(x,y,z));

        model = glm::scale(model, glm::vec3(scaleFactor));


        return model;
    }

    bool isPoint() const {
        return m_isPoint;
    }



private:

    float m_xCoord;
    float m_yCoord{0.5f};
    float m_zCoord{-8.0f};
    bool m_isPoint;
    float defaultPosition {-8.0f};


};



#endif //PROJECT_BASE_CUBE_H
