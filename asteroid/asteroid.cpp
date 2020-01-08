
#include <algorithm>
#include <string>
#include "olcConsoleGameEngine.h"

class AsteroidGame : public olcConsoleGameEngine
{
public:
    AsteroidGame()
    {
        m_sAppName = L"AsteroidGame";
    }

private:
    void wrapper(float x, float y, float& ox, float& oy)
    {
        ox = x;
        oy = y;
        if (x < 0.0f)
            ox = x + (float)ScreenWidth();
        if (x > (float)ScreenWidth())
            ox = x - (float)ScreenWidth();

        if (y < 0.0f)
            oy = y + (float)ScreenHeight();
        else if (y > (float)ScreenHeight())
            oy = y - (float)ScreenHeight();
    };

    void DrawWireFormModel(vector<pair<float, float> > modelCoordinates, float x, float y, float r, float s, short col = FG_WHITE)
    {
        vector<pair<float, float>> transformedModelCoordinates;
        int size = modelCoordinates.size();
        transformedModelCoordinates.resize(size);

        float rx[3];
        float ry[4];
        // rotate model
        for (int i = 0; i < size; i++) {
            transformedModelCoordinates[i].first = modelCoordinates[i].first * cosf(r) - modelCoordinates[i].second * sinf(r);
            transformedModelCoordinates[i].second = modelCoordinates[i].first * sinf(r) + modelCoordinates[i].second * cosf(r);
        }

        // scale
        for (int i = 0; i < size; i++) {
            transformedModelCoordinates[i].first = s * transformedModelCoordinates[i].first;
            transformedModelCoordinates[i].second = s * transformedModelCoordinates[i].second;
        }

        // translate
        for (int i = 0; i < size; i++) {
            transformedModelCoordinates[i].first = transformedModelCoordinates[i].first + x;
            transformedModelCoordinates[i].second = transformedModelCoordinates[i].second + y;
        }

        // draw ship
        for (int i = 0; i < size + 1; i++)
            DrawLine(transformedModelCoordinates[i % size].first, transformedModelCoordinates[i % size].second, 
                transformedModelCoordinates[(i + 1) % size].first, transformedModelCoordinates[(i + 1) % size].second, 
                PIXEL_SOLID, col);
    }

    virtual void Draw(int x, int y, wchar_t c = 0x2588, short col = 0x000F)
    {
        float tx = x;
        float ty = y;
        wrapper(x, y, tx, ty);
        olcConsoleGameEngine::Draw(tx, ty, c, col);
    }

    bool IsInCircle(float px, float py, float cx, float cy, float r)
    {
        return sqrtf(powf(px - cx, 2) + powf(py - cy, 2)) < r;
    }

    void Reset()
    {
        vectAsteroid.clear();
        vectAsteroid.push_back({ 10.0f, 30.0f, 8.0f, -6.0f, 16, 0.0f });

        ship.x = 20.0f;
        ship.y = 60.0f;
        ship.dx = 0.0f;
        ship.dy = 0.0f;
        ship.size = 1;
        ship.angle = 0.0f;

        bDead = false;
        nScore = 0;
    }

protected:
    struct sSpaceObject 
    {
        float x;
        float y;
        float dx;
        float dy;
        int size;
        float angle;
    };

    vector<sSpaceObject> vectAsteroid;
    list<sSpaceObject> lBullets;

    vector<pair<float, float>> vectShipModel;
    vector<pair<float, float>> vectAsteroidModel;

    sSpaceObject ship;
    bool bDead;
    int nScore;

    sSpaceObject CreateNewAsteroids(sSpaceObject ship)
    {
        float angle = (float)rand() / (float)RAND_MAX * 4.188f + 1.047f;
        float distance = (float)rand() / (float)RAND_MAX * 24.0f + 16.0f;
        float dx = (float)rand() / (float)RAND_MAX * 20.0f - 10.0f;
        float dy = (float)rand() / (float)RAND_MAX * 20.0f - 10.0f;
        float x = distance * sinf(ship.angle + angle) + ship.x;
        float y = distance * cosf(ship.angle + angle) + ship.y;
        return { x, y, dx, dy, 16, 0.0f };
    }

    bool OnUserCreate()
    {
        Reset();

        vectShipModel = {
            {0.0f, -5.0f},
            {-2.5f, 2.5f},
            {2.5f, 2.5f}
        };

        for (int i = 0; i < 20; i++) {
            float a = 2.0f * 3.1415926f * (float)i / 20.0f;
            float r = (float)rand() / (float)RAND_MAX * 0.4f + 0.8f;
            vectAsteroidModel.push_back(make_pair(r * sinf(a), r * cosf(a)));
        }

        return true;
    }

    bool OnUserUpdate(float fElapsedTime)
    {
        if (bDead) {
            Reset();
        }

        // input
        if (m_keys[VK_LEFT].bHeld) {
            ship.angle -= 5.0f * fElapsedTime;
        }
        if (m_keys[VK_RIGHT].bHeld) {
            ship.angle += 5.0f * fElapsedTime;
        }
        if (m_keys[VK_UP].bHeld) {
            ship.dx += 20.0f * sinf(ship.angle) * fElapsedTime;
            ship.dy -= 20.0f * cosf(ship.angle) * fElapsedTime;
        }

        if (m_keys[VK_SPACE].bPressed) {
            sSpaceObject bullet = {ship.x, ship.y, ship.dx + 20.0f * sinf(ship.angle), ship.dy - 20.0f * cosf(ship.angle), 1, ship.angle};
            lBullets.push_back(bullet);
        }

        // ship move
        ship.x += ship.dx * fElapsedTime;
        ship.y += ship.dy * fElapsedTime;
        wrapper(ship.x, ship.y, ship.x, ship.y);

        vector<sSpaceObject> vectNewAsteroids;
        for (auto& i : lBullets) {
            i.x += i.dx * fElapsedTime;
            i.y += i.dy * fElapsedTime;

            for (auto& a : vectAsteroid) {
                // collision detection
                if (IsInCircle(i.x, i.y, a.x, a.y, a.size)) {
                    if (a.size > 4) {
                        sSpaceObject newAsteroid1 = { a.x, a.y, -a.dx, a.dy, a.size / 2, 0.0f};
                        sSpaceObject newAsteroid2 = { a.x, a.y, a.dx, -a.dy, a.size / 2, 0.0f };
                        vectNewAsteroids.push_back(newAsteroid1);
                        vectNewAsteroids.push_back(newAsteroid2);
                    }
                    // increase score
                    nScore += 100;
                    // remove bullet
                    i.x = -100.0f;
                    // remove origin asteroid
                    a.x = -100.0f;
                }
            }
        }

        // remove invalid asteroids
        auto it = remove_if(vectAsteroid.begin(), vectAsteroid.end(), [&](sSpaceObject obj) {return obj.x < 0.0f; });
        if (it != vectAsteroid.end()) {
            vectAsteroid.erase(it);
        }

        // add new asteroids
        for (auto i : vectNewAsteroids) {
            vectAsteroid.push_back(i);
        }
        vectNewAsteroids.clear();

        if (vectAsteroid.empty()) {
            nScore += 1000;
            // create new asteroids
            sSpaceObject newAsteroid1 = CreateNewAsteroids(ship);
            sSpaceObject newAsteroid2 = CreateNewAsteroids(ship);
            vectAsteroid.push_back(newAsteroid1);
            vectAsteroid.push_back(newAsteroid2);
        }

        for (auto& i : vectAsteroid) {
            i.x += i.dx * fElapsedTime;
            i.y += i.dy * fElapsedTime;
            i.angle += 0.3f * fElapsedTime;
            wrapper(i.x, i.y, i.x, i.y);
            if (IsInCircle(ship.x, ship.y, i.x, i.y, i.size)) {
                bDead = true;
            }
        }

        // clear screen
        Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, 0);

        // draw asteroid
        for (auto& i : vectAsteroid) {
            DrawWireFormModel(vectAsteroidModel, i.x, i.y, i.angle, i.size, FG_YELLOW);
        }

        // draw ship
        DrawWireFormModel(vectShipModel, ship.x, ship.y, ship.angle, 1.0f);

        // remove bullet out of screen
        if (!lBullets.empty()) {
            auto i = remove_if(lBullets.begin(), lBullets.end(), [&](sSpaceObject obj) {return obj.x < 0.0f || obj.x >(float)ScreenWidth() - 1.0f || obj.y < 0.0f || obj.y >(float)ScreenHeight() - 1.0f;});
            if (i != lBullets.end()) {
                lBullets.erase(i);
            }
        }

        // draw bullets
        for (auto& i : lBullets) {
            Draw(i.x, i.y, PIXEL_SOLID, FG_WHITE);
        }

        // display score
        DrawString(2, 2, L"Score:" + to_wstring(nScore));

        return true;
    }
};

int main() 
{
    AsteroidGame game;
    game.ConstructConsole(160, 100, 8, 8);

    game.Start();
    return 0;
}