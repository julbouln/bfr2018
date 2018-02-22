#include <SFML/Graphics.hpp>

struct TileVertex {
    sf::Vertex quad0, quad1, quad2, quad3;
};

class TileLayer : public sf::Drawable, public sf::Transformable
{
public:
    int width;
    int height;
    sf::Color color;

    TileLayer() {
        this->width = 0;
        this->height = 0;
    }
    TileLayer(int w, int h) {
        this->width = w;
        this->height = h;
    }

    void addTileRect(sf::IntRect rect) {
        tileRects.push_back(rect);
    }

    void init(sf::Texture *tex) {
        m_tileset = tex;
        m_vertices.setPrimitiveType(sf::Quads);
        if(width)
            m_vertices.resize(width * height * 4);
        color = sf::Color(0xff,0xff,0xff,0xff);
    }

    void init(sf::Texture *tex, sf::Color col) {
        m_tileset = tex;
        m_vertices.setPrimitiveType(sf::Quads);
        if(width)
            m_vertices.resize(width * height * 4);
        color = col;
    }

    void clear() {
        m_vertices.clear();
    }

    inline int index(int x, int y) const { return x + width * y; }

    TileVertex createTileVertex(int tile, sf::Vector2i pos, sf::Color vcol) {
        sf::IntRect tileRect = tileRects[tile];
        TileVertex tv;
        // define its 4 corners
        tv.quad0.position = sf::Vector2f(pos.x * tileRect.width, pos.y * tileRect.height);
        tv.quad1.position = sf::Vector2f((pos.x + 1) * tileRect.width, pos.y * tileRect.height);
        tv.quad2.position = sf::Vector2f((pos.x + 1) * tileRect.width, (pos.y + 1) * tileRect.height);
        tv.quad3.position = sf::Vector2f(pos.x * tileRect.width, (pos.y + 1) * tileRect.height);

        // define its 4 texture coordinates
        tv.quad0.texCoords = sf::Vector2f(tileRect.left, tileRect.top);
        tv.quad1.texCoords = sf::Vector2f(tileRect.left + tileRect.width, tileRect.top);
        tv.quad2.texCoords = sf::Vector2f(tileRect.left + tileRect.width, tileRect.top + tileRect.height);
        tv.quad3.texCoords = sf::Vector2f(tileRect.left, tileRect.top + tileRect.height);

        tv.quad0.color = vcol;
        tv.quad1.color = vcol;
        tv.quad2.color = vcol;
        tv.quad3.color = vcol;

        return tv;
    }

    void setPosition(int tile, sf::Vector2i pos) {
        int idx = this->index(pos.x, pos.y) * 4;

        TileVertex tv = this->createTileVertex(tile, pos, color);

        m_vertices[idx] = tv.quad0;
        m_vertices[idx+1] = tv.quad1;
        m_vertices[idx+2] = tv.quad2;
        m_vertices[idx+3] = tv.quad3;
    }


    void clearPosition(sf::Vector2i pos) {
        int idx = this->index(pos.x, pos.y) * 4;

        TileVertex tv = this->createTileVertex(0, pos, sf::Color(0xff,0xff,0xff,0x00));

        m_vertices[idx] = tv.quad0;
        m_vertices[idx+1] = tv.quad1;
        m_vertices[idx+2] = tv.quad2;
        m_vertices[idx+3] = tv.quad3;
    }

    void addPosition(int tile, sf::Vector2i pos) {
//        std::cout << "ADD POS "<<tile<<" "<<tileRects.size()<<std::endl;

        TileVertex tv = this->createTileVertex(tile, pos, color);

        m_vertices.append(tv.quad0);
        m_vertices.append(tv.quad1);
        m_vertices.append(tv.quad2);
        m_vertices.append(tv.quad3);
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

    void resize(int size, int width, int height) {
        this->layers.clear();
        while(this->layers.size() < size) {
            this->layers.push_back(TileLayer(width, height));
        }
    }

};