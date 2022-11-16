#include <iostream>
#include <string>
#include <sstream>
#include <iostream>
#include <Windows.h>

#include "Binary.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma comment(lib, "lib/openal32.lib")

#include <AL/al.h>
#include <AL/alc.h>
#include <AudioFile/AudioFile.h>

#define OpenAL_ErrorCheck(message)\
{\
    ALenum error = alGetError();\
    if(error != AL_NO_ERROR)\
    {\
        std::cerr << "OpenAl error: " << error << " with call for" << #message << std::endl;\
    }\
}

#define alec(FUNCTION_CALL)\
FUNCTION_CALL;\
OpenAL_ErrorCheck(FUNCTION_CALL);

using namespace std;


/* Прототипы функций */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void program_create(GLuint program, GLuint vertexShader, GLuint fragmentShader, GLuint geometryShader = 0);
void draw_circle(GLfloat Xvertex, GLfloat Yvertex, GLfloat radius, glm::vec3 color);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);


const GLuint WIDTH = 800, HEIGHT = 600;

/* Шейдеры */
//Вершинные шейдеры
const GLchar* defaultShader = "#version 460 core\n"
    "layout (location = 0) in vec3 position;\n"
    "void main()\n"
    "{\n"
    "gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
    "}\0";

const GLchar* vertexSunMoon = "#version 460 core\n"
    "layout(location = 0) in vec3 position;\n"
    "uniform mat4 transform;\n"
    "void main()\n"
    "{\n"
    "gl_Position = transform * vec4(position, 1.0f);\n"
    "}\n\0";

const GLchar* vertexRainDrops = "#version 460 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aOffset;\n"
    "vec4 result;\n"
    "void main()\n"
    "{\n"
    "result = vec4(aPos + aOffset, 0.0, 1.0);\n"
    "gl_Position = result;\n"
    "};\n\0";

const GLchar* vertexHumans = "#version 460 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "gl_Position = vec4(aPos, 1.0);\n"
    "};\n\0";

const GLchar* vertexHumansHands = "#version 460 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 transform;\n"
    "void main()\n"
    "{\n"
    "gl_Position = transform * vec4(aPos, 1.0);\n"
    "};\n\0";

const GLchar* vertexGeometryEntity = "#version 460 core\n"
    "layout (location = 0) in vec3 position;"
    "layout (location = 1) in vec3 colorS;\n"
    "out vec4 color;\n"
    "void main ()\n"
    "{\n"
    "gl_Position = vec4(position, 1.0f);\n"
    "color = vec4(colorS, 1.0f);\n"
    "}\n\0";


//Фрагментные шейдеры (они же цветовые)
const GLchar* fragmentShaderBrown = "#version 460 core\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "color = vec4(0.43f, 0.26f, 0.09f, 1.0f);\n"
    "}\n\0";

const GLchar* fragmentShaderPurple = "#version 460 core\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "color = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
    "}\n\0";

const GLchar* fragmentShaderBlue = "#version 460 core\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "color = vec4(0.0f, 0.0f, 0.65f, 1.0f);\n"
    "}\n\0";

const GLchar* fragmentShaderBlack = "#version 460 core\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "color = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
    "}\n\0";

const GLchar* fragmentShaderGeometryEntity = "#version 460 core\n"
    "in vec4 color;\n"
    "out vec4 colorS;\n"
    "void main()\n"
    "{\n"
    "colorS = color;\n"
    "}\n\0";



bool flag = false; //Флаг на открытие приложения из плеера

int main(void)
{
    createBinaryFile();
    std::string ghost = readFromBinaryFile();

    cout << ghost << endl;

    std::cout << __cplusplus << endl << endl;



    /* Инициализируем библиотеку GLFW */
    if (!glfwInit()) {
        return -1;
    }

    /* Выключаем возможность изменения размера окна */
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* player;
    player = glfwCreateWindow(400, 250, "Player", NULL, NULL);
    glfwMakeContextCurrent(player);


    glfwSetCursorPosCallback(player, cursor_position_callback);
    glfwSetKeyCallback(player, key_callback);
    glfwSetMouseButtonCallback(player, mouse_button_callback);

    while (!glfwWindowShouldClose(player)) {

        glfwPollEvents();

        /* Рендерим само окно */
        glClearColor(0.41f, 0.0f, 0.25, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);



        int state = glfwGetMouseButton(player, GLFW_MOUSE_BUTTON_LEFT);
        if (state == GLFW_PRESS) {
            cout << "Click!" << endl;
        }

        if (ghost == "Kursach.exe\0" && flag == true) {

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /////////////////////////// AUDIO
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            const ALCchar* defaultDeviceString = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER); //Определяем устройство по умолчанию если есть, если нет - nullptr
            ALCdevice* device = alcOpenDevice(defaultDeviceString); //Открываем наш аудиопоток

            if (!device) { //Проверяем аудиопоток на открытие
                cout << "Failed to load default device for OpenAL" << endl;
            }

            //Выводим спецификатор при помощи которого воспроизводится звук
            cout << "Default audio: " << alcGetString(device, ALC_DEVICE_SPECIFIER) << endl;


            //Создаём аудио-контекст
            ALCcontext* context = alcCreateContext(device, /*attrlist*/ nullptr);
            //OpenAL_ErrorCheck(context);

            if (!alcMakeContextCurrent(context)) {
                cout << "Failed to make context current" << endl;
                return -1;
            }

            alec(alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f));
            alec(alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f));
            ALfloat forwardAndUpVectors[] = {
                1.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f
            };
            alec(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));

            AudioFile<float> backgroundmusic;
            if (!backgroundmusic.load("sounds/tihiy-dojdik.wav")) {
                cout << "Error load background music" << endl;
                return -1;
            };

            vector<uint8_t> backgroundmusicPCMDataBytes;
            backgroundmusic.writePCMToBuffer(backgroundmusicPCMDataBytes);

            auto convertFileToOpenALFormat = [](const AudioFile<float>& audioFile) {
                int bitDepth = audioFile.getBitDepth();
                if (bitDepth == 16)
                    return audioFile.isStereo() ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
                else if (bitDepth == 8)
                    return audioFile.isMono() ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
                else
                    return -1;
            };

            ALuint monoSoundBuffer;
            alec(alGenBuffers(1, &monoSoundBuffer));
            alec(alBufferData(monoSoundBuffer, convertFileToOpenALFormat(backgroundmusic), backgroundmusicPCMDataBytes.data(), backgroundmusicPCMDataBytes.size(), backgroundmusic.getSampleRate()));



            ALuint monoSource;
            alec(alGenSources(1, &monoSource));
            alec(alSource3f(monoSource, AL_POSITION, 0.0f, 0.0f, 0.0f));
            alec(alSource3f(monoSource, AL_VELOCITY, 0.0f, 0.0f, 0.0f));
            alec(alSourcef(monoSource, AL_PITCH, 1.f));
            alec(alSourcef(monoSource, AL_GAIN, 1.f));
            alec(alSourcei(monoSource, AL_LOOPING, AL_TRUE));
            alec(alSourcei(monoSource, AL_BUFFER, monoSoundBuffer));

            alec(alSourcePlay(monoSource));

            ALint SourceState; //Статус проигрывания









            AudioFile<float> BS;
            if (!BS.load("sounds/zozh_no_gachi.wav")) {
                cout << "Error load background music" << endl;
                return -1;
            };

            vector<uint8_t> BSPCMDataBytes;
            BS.writePCMToBuffer(BSPCMDataBytes);

            ALuint BSSoundBuffer;
            alec(alGenBuffers(1, &BSSoundBuffer));
            alec(alBufferData(BSSoundBuffer, convertFileToOpenALFormat(BS), BSPCMDataBytes.data(), BSPCMDataBytes.size(), BS.getSampleRate()));


            int Gsize = BSPCMDataBytes.size();
            cout << "Second track full bites size: " << Gsize << endl;
            int step = Gsize / 20;

            ALuint BSSource;
            alec(alGenSources(1, &BSSource));
            alec(alSource3f(BSSource, AL_POSITION, 0.0f, 0.0f, 0.0f));
            alec(alSource3f(BSSource, AL_VELOCITY, 0.0f, 0.0f, 0.0f));
            alec(alSourcef(BSSource, AL_PITCH, 1.f));
            alec(alSourcef(BSSource, AL_GAIN, 0.1f));
            alec(alSourcei(BSSource, AL_LOOPING, AL_FALSE));
            alec(alSourcei(BSSource, AL_BUFFER, BSSoundBuffer));

            alec(alSourcePlay(BSSource));

            ALint SourceState2;
            ALint SourceBitsProcessed; //Кол-во проигранных бит










            //////////////////////////////////////////////////////////////////////////////////////////
            //////////// GRAPHICS
            /////////////////////////////////////////////////////////////////////////////////////////

            /* Инициализируем указатель на окно */
            GLFWwindow* window;

            /* Создаём окно и передаём ему OpenGL контекст */
            window = glfwCreateWindow(640, 480, "Chill", NULL, NULL);

            if (!window)
            {
                glfwTerminate();
                return -1;
            }

            /* Выдаём контекст текущему окну */
            glfwMakeContextCurrent(window);

            /*Привязка функции закрытия окна на escape*/
            glfwSetKeyCallback(window, key_callback);

            //Даём знать GLEW как использовать современный подход к получению указателей на функции и расширений.
            glewExperimental = GL_TRUE;


            // Инициализируем GLEW для настройки указателей на функции OpenGL
            glewInit();

            // Определяем размеры видимой области
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);



            GLuint vertexshaderDEFAULT = glCreateShader(GL_VERTEX_SHADER); //Стандартный шейдер (для земли)
            GLuint vertexsunmoon = glCreateShader(GL_VERTEX_SHADER); //Шейдер для вращения солнца и луны
            GLuint vertexraindrops = glCreateShader(GL_VERTEX_SHADER); //Шейдер для капель
            GLuint vertexhumans = glCreateShader(GL_VERTEX_SHADER); //Шейдер для людей
            GLuint vertexhands = glCreateShader(GL_VERTEX_SHADER); //Шейдер для рук
            GLuint vertexgeometryentity = glCreateShader(GL_VERTEX_SHADER); //Шейдер для геометрических сущностей

            GLuint fragmentshaderBrown = glCreateShader(GL_FRAGMENT_SHADER);
            GLuint fragmentshaderBlue = glCreateShader(GL_FRAGMENT_SHADER);
            GLuint fragmentshaderPurple = glCreateShader(GL_FRAGMENT_SHADER);
            GLuint fragmentshaderBlack = glCreateShader(GL_FRAGMENT_SHADER);
            GLuint fragmentgeometryentity = glCreateShader(GL_FRAGMENT_SHADER);

            GLuint shaderProgramForGround = glCreateProgram();
            GLuint shaderProgramForFloor = glCreateProgram();
            GLuint shaderProgramForRaindrops = glCreateProgram();
            GLuint shaderProgramForHumans = glCreateProgram();
            GLuint shaderProgramForHands = glCreateProgram();
            GLuint shaderProgramForGeometry = glCreateProgram();




            glShaderSource(vertexshaderDEFAULT, 1, &defaultShader, NULL);
            glCompileShader(vertexshaderDEFAULT);

            glShaderSource(vertexsunmoon, 1, &vertexSunMoon, NULL);
            glCompileShader(vertexsunmoon);

            glShaderSource(vertexraindrops, 1, &vertexRainDrops, NULL);
            glCompileShader(vertexraindrops);

            glShaderSource(vertexhumans, 1, &vertexHumans, NULL);
            glCompileShader(vertexhumans);

            glShaderSource(vertexhands, 1, &vertexHumansHands, NULL);
            glCompileShader(vertexhands);

            glShaderSource(vertexgeometryentity, 1, &vertexGeometryEntity, NULL);
            glCompileShader(vertexgeometryentity);





            glShaderSource(fragmentshaderBrown, 1, &fragmentShaderBrown, NULL);
            glCompileShader(fragmentshaderBrown);

            glShaderSource(fragmentshaderPurple, 1, &fragmentShaderPurple, NULL);
            glCompileShader(fragmentshaderPurple);

            glShaderSource(fragmentshaderBlue, 1, &fragmentShaderBlue, NULL);
            glCompileShader(fragmentshaderBlue);

            glShaderSource(fragmentshaderBlack, 1, &fragmentShaderBlack, NULL);
            glCompileShader(fragmentshaderBlack);

            glShaderSource(fragmentgeometryentity, 1, &fragmentShaderGeometryEntity, NULL);
            glCompileShader(fragmentgeometryentity);


            program_create(shaderProgramForFloor, vertexsunmoon, fragmentshaderPurple);
            program_create(shaderProgramForGround, vertexshaderDEFAULT, fragmentshaderBrown);
            program_create(shaderProgramForHumans, vertexhumans, fragmentshaderBlack);
            program_create(shaderProgramForRaindrops, vertexraindrops, fragmentshaderBlue);
            program_create(shaderProgramForHands, vertexhands, fragmentshaderBlack);
            program_create(shaderProgramForGeometry, vertexgeometryentity, fragmentgeometryentity);




            GLint success;
            GLchar infoLog[512];
            glGetShaderiv(vertexraindrops, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(vertexraindrops, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
            }


            glm::vec2 translations[200];
            glm::vec2 translationsNext[200];
            int index = 0;
            float offset = 0.020f;
            for (int x = 0; x < 200; x++)
            {
                glm::vec2 translation;
                translation.x = (float)(x / 200.0) * 2 + offset;
                translation.y = -(float)(rand() % 100 / 110.0);
                translations[x] = translation;
                translationsNext[x] = translation;
            }


            GLfloat floor[] = {
                //Вершины         
                -1.0f, -1.0f, 0.0f,
                -1.0f, -0.5f, 0.0f,
                1.0f, -1.0f, 0.0f,
                1.0f, -0.5f, 0.0f
            };

            GLuint floorIndexes[] = {
                0, 1, 2,
                2, 3, 1
            };

            GLfloat gym[] = {

                //Левая стенка
                -0.5f, -0.5f, 0.0f,
                -0.45f, -0.5f, 0.0f,
                -0.45f, 0.5f, 0.0f,
                -0.5f, 0.5f, 0.0f,

                //Потолок
                0.5f, 0.5f, 0.0f,
                0.5f, 0.45f, 0.0f,
                -0.5f, 0.45f, 0.0f,

                //Правая стенка
                0.45f, 0.5f, 0.0f,
                0.5f, -0.35f, 0.0f,
                0.45f, -0.35f, 0.0f,

                //Левый пролёт
                -0.5f, 0.025f, 0.0f,
                -0.5f, -0.025f, 0.0f,
                0.0f, 0.025f, 0.0f,
                0.0f, -0.025f, 0.0f,

                //Правый пролёт
                0.5f, 0.025f, 0.0f,
                0.5f, -0.025f, 0.0f,
                0.1f, 0.025f, 0.0f,
                0.1f, -0.025f, 0.0f
            };

            GLuint gymindexes[] = {
                0, 1, 2,
                2, 3, 0,
                3, 4, 5,
                6, 3, 5,
                4, 7, 8,
                8, 9, 7,

                10, 11, 12,
                12, 13, 11,

                14, 15, 16,
                17, 16, 15
            };


            GLfloat raindrops[] = {
                -0.99f, 2.0f, 0.0f,
                -1.0f, 1.95f, 0.0f,
            };


            GLfloat Kirill[] = {
                //Левая нога
                -0.30f, -0.5f, 0.0f,
                -0.28f, -0.4f, 0.0f,

                //Правая нога
                -0.28f, -0.4f, 0.0f,
                -0.26f, -0.5f, 0.0f,

                //Тело
                -0.28f, -0.4f, 0.0f,
                -0.28f, -0.3f, 0.0f,

                //Правое плечо
                -0.28f, -0.32f, 0.0f,
                -0.23f, -0.32f, 0.0f,

                //Правая кисть
                -0.23f, -0.32f, 0.0f,
                -0.23f, -0.22f, 0.0f,

                //Левое плечо
                -0.28f, -0.32f, 0.0f,
                -0.33f, -0.32f, 0.0f,

                //Левая кисть
                -0.33f, -0.32f, 0.0f,
                -0.33f, -0.22f, 0.0f,

                //Гантеля
                -0.37f, -0.22f, 0.0f,
                -0.19f, -0.22f, 0.0f
            };

            GLfloat stolb[] = {
                0.0f, -0.26f, 0.0f,
                0.0f, -0.5f, 0.0f
            };

            GLuint VBOs[6],
                VAOs[4],
                EBOs[2];

            glGenBuffers(6, VBOs);
            glGenVertexArrays(4, VAOs);
            glGenBuffers(2, EBOs);



            glBindVertexArray(VAOs[0]);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndexes), floorIndexes, GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(floor), floor, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);





            glBindVertexArray(VAOs[1]);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[1]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gymindexes), gymindexes, GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(gym), gym, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);





            glBindVertexArray(VAOs[2]);

            glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(raindrops), raindrops, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(translations), translations, GL_STATIC_DRAW);

            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*)0);
            glVertexAttribDivisor(1, 1);
            glEnableVertexAttribArray(1);

            glBindVertexArray(0);




            glBindVertexArray(VAOs[3]);

            glBindBuffer(GL_ARRAY_BUFFER, VBOs[4]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(Kirill), Kirill, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);


            GLfloat muscle_status = 2.0f;

            /* Проверяем флаг на закрытие окна */
            while (!glfwWindowShouldClose(window))
            {
                /* Poll for and process events */
                glfwPollEvents();


                alec(alGetSourcei(monoSource, AL_SOURCE_STATE, &SourceState)); //Статус 1 трека
                alec(alGetSourcei(BSSource, AL_SOURCE_STATE, &SourceState2)); //Статус 2 трека

                alec(alGetSourcei(BSSource, AL_BYTE_OFFSET, &SourceBitsProcessed)); //Количество проигранных бит текущего трека


                if (SourceState2 != AL_PLAYING) {
                    cout << "END!" << endl;
                    break;
                }

                GLfloat timeValue = glfwGetTime();
                GLfloat blueValue = (sin(timeValue) / 2) + 0.5;

                /* Рендерим само окно */
                glClearColor(0.41f, 0.0f, 0.58 + blueValue, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);






                /* Первая программа, отвечающая за землю */
                glUseProgram(shaderProgramForGround);

                glBindVertexArray(VAOs[0]);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

                glBindVertexArray(0);











                /* Вторая программа, отвечающая за спортивный зал */
                glUseProgram(shaderProgramForFloor);

                glm::mat4 transform;

                GLint transformLoc = glGetUniformLocation(shaderProgramForFloor, "transform");
                glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

                glBindVertexArray(VAOs[1]);
                glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);

                glBindVertexArray(0);








                glLineWidth(1);

                /* Третья программа, отвечающая за дождь */
                glUseProgram(shaderProgramForRaindrops);

                for (int z = 0; z < 200; z++) {
                    if (translationsNext[z].y > -2.45f && translationsNext[z].x < 0.5f) {
                        translationsNext[z].y = translationsNext[z].y - 0.002f;
                    }
                    else if (translationsNext[z].y > -1.45f && translationsNext[z].x > 0.5f && translationsNext[z].x < 1.5f) {
                        translationsNext[z].y = translationsNext[z].y - 0.002f;
                    }
                    else if (translationsNext[z].y > -2.45f && translationsNext[z].x > 1.5f) {
                        translationsNext[z].y = translationsNext[z].y - 0.002f;
                    }
                    else {
                        translationsNext[z].y = translations[z].y;
                    }
                }

                glBindVertexArray(VAOs[2]);

                glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(translationsNext), translationsNext, GL_STATIC_DRAW);


                glDrawArraysInstanced(GL_LINES, 0, 2, 200);

                glBindVertexArray(0);





                if (SourceBitsProcessed > step && muscle_status < 8.0f) {
                    step = step + step;
                    muscle_status = muscle_status + 0.5f;
                }

                glLineWidth(muscle_status);
                GLfloat hands = sin(timeValue) / 5;

                /* Отрисовка людей */
                glUseProgram(shaderProgramForHumans);

                //Правая рука
                Kirill[22] = -0.32f * (1 - hands);
                Kirill[25] = -0.32f * (1 - hands);
                Kirill[28] = -0.22f * (1 - hands);

                //Левая рука
                Kirill[34] = -0.32f * (1 - hands);
                Kirill[37] = -0.32f * (1 - hands);
                Kirill[40] = -0.22f * (1 - hands);

                //Гантеля
                Kirill[43] = -0.22f * (1 - hands);
                Kirill[46] = -0.22f * (1 - hands);
                draw_circle(-0.17f, -0.22f * (1 - hands), 0.02f * (muscle_status / 2), glm::vec3(0.0f, 0.0f, 0.0f));
                draw_circle(-0.39f, -0.22f * (1 - hands), 0.02f * (muscle_status / 2), glm::vec3(0.0f, 0.0f, 0.0f));

                glBindVertexArray(VAOs[3]);

                glBindBuffer(GL_ARRAY_BUFFER, VBOs[4]);
                glBufferData(GL_ARRAY_BUFFER, sizeof(Kirill), Kirill, GL_STATIC_DRAW);

                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
                glEnableVertexAttribArray(0);



                glDrawArrays(GL_LINES, 0, 16);
                glBindVertexArray(0);



                glLineWidth(2);


                glUseProgram(shaderProgramForGeometry);
                draw_circle(-0.28f, -0.27f, 0.03f, glm::vec3(0.0f, 0.0f, 0.0f));
                draw_circle(0.0f, -0.2f, 0.06f, glm::vec3(0.18f, 0.83f, 0.78f));


                glBufferData(GL_ARRAY_BUFFER, sizeof(stolb), stolb, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

                glDrawArrays(GL_LINES, 0, 2);

                glBindVertexArray(0);




                /* Меняем первый и второй буффер */
                glfwSwapBuffers(window);
            }

            glDeleteShader(vertexshaderDEFAULT);
            glDeleteShader(vertexsunmoon);
            glDeleteShader(vertexhumans);
            glDeleteShader(vertexraindrops);
            glDeleteShader(vertexgeometryentity);

            glDeleteShader(fragmentshaderBrown);
            glDeleteShader(fragmentshaderPurple);
            glDeleteShader(fragmentgeometryentity);
            glDeleteShader(fragmentshaderPurple);
            glDeleteShader(fragmentshaderBlack);

            alDeleteSources(1, &monoSource);
            alDeleteSources(1, &BSSource);
            alDeleteBuffers(1, &monoSoundBuffer);
            alDeleteBuffers(1, &BSSoundBuffer);

            /* Выдаём контекст обратно плееру и завершаем процесс OpenGL */
            glfwDestroyWindow(window);
            glfwMakeContextCurrent(player);

            flag = false;

            cout << "Application closed!" << endl;
            printf("Compiled for GLFW version %i.%i.%i\n", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
        }

        glfwSwapBuffers(player);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        flag = true;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        cout << "Click!" << endl;
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {

}

//Создаём программу и проверяем её статус
void program_create(GLuint program, GLuint vertexShader, GLuint fragmentShader, GLuint geometryShader) {

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    if (geometryShader != 0) {
        glAttachShader(program, geometryShader);
    }
    glLinkProgram(program);

    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}

void draw_circle(GLfloat Xvertex, GLfloat Yvertex, GLfloat radius, glm::vec3 color) {
    GLuint Circle_VBO, Color_VBO;
    glm::vec3 vertices[100];

    float a = 3.14 * 2 / 50;

    for (int z = 0; z < 100; z = z + 2) {
        vertices[z].x = sin(a * z) * radius + Xvertex;
        vertices[z].y = cos(a * z) * radius + Yvertex;
        vertices[z].z = 0.0f;
        vertices[z + 1].x = color.x;
        vertices[z + 1].y = color.y;
        vertices[z + 1].z = color.z;
    }

    glGenBuffers(1, &Circle_VBO);
    glGenBuffers(1, &Color_VBO);

    glBindBuffer(GL_ARRAY_BUFFER, Circle_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 50);
}