#include <SFML/Graphics.hpp>

class TileLayer : public sf::Drawable, public sf::Transformable
{
public:
    sf::Color color;

    void addTileRect(sf::IntRect rect) {
        tileRects.push_back(rect);
    }

    void init(sf::Texture *tex) {
        m_tileset = tex;
        m_vertices.setPrimitiveType(sf::Quads);
//        m_vertices.resize(width * height * 4);
        color = sf::Color(0xff,0xff,0xff,0xff);
    }

    void init(sf::Texture *tex, sf::Color col) {
        m_tileset = tex;
        m_vertices.setPrimitiveType(sf::Quads);
//        m_vertices.resize(width * height * 4);
        color = col;
    }

    void clear() {
        m_vertices.clear();
    }

    void addPosition(int tile, sf::Vector2i pos) {
//        std::cout << "ADD POS "<<tile<<" "<<tileRects.size()<<std::endl;
        sf::IntRect tileRect = tileRects[tile];

//        sf::Vertex* quad = &m_vertices[(pos.x + pos.y * width) * 4];
        sf::Vertex quad0, quad1, quad2, quad3;

        // define its 4 corners
        quad0.position = sf::Vector2f(pos.x * tileRect.width, pos.y * tileRect.height);
        quad1.position = sf::Vector2f((pos.x + 1) * tileRect.width, pos.y * tileRect.height);
        quad2.position = sf::Vector2f((pos.x + 1) * tileRect.width, (pos.y + 1) * tileRect.height);
        quad3.position = sf::Vector2f(pos.x * tileRect.width, (pos.y + 1) * tileRect.height);

        // define its 4 texture coordinates
        quad0.texCoords = sf::Vector2f(tileRect.left, tileRect.top);
        quad1.texCoords = sf::Vector2f(tileRect.left + tileRect.width, tileRect.top);
        quad2.texCoords = sf::Vector2f(tileRect.left + tileRect.width, tileRect.top + tileRect.height);
        quad3.texCoords = sf::Vector2f(tileRect.left, tileRect.top + tileRect.height);

        quad0.color = color;
        quad1.color = color;
        quad2.color = color;
        quad3.color = color;

        m_vertices.append(quad0);
        m_vertices.append(quad1);
        m_vertices.append(quad2);
        m_vertices.append(quad3);
    }

private:

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // apply the transform
        states.transform *= getTransform();

        // apply the tileset texture
        states.texture = m_tileset;

        // draw the vertex array
        target.draw(m_vertices, states);
    }

    sf::VertexArray m_vertices;
    sf::Texture *m_tileset;

    std::vector<sf::IntRect> tileRects;
//    std::map<EntityID, sf::IntRect> tileRects;
};

class TileMap {
public:
    std::vector<TileLayer> layers;

    void resize(int size) {
        this->layers.clear();
        while(this->layers.size() < size) {
            this->layers.push_back(TileLayer());
        }
    }
};