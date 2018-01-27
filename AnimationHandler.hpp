#ifndef ANIMATION_HANDLER_HPP
#define ANIMATION_HANDLER_HPP

#include <SFML/Graphics.hpp>
#include <vector>

enum class AnimationType {
    Static,
    Timer
};

class Animation {
public:
    AnimationType type;
    std::vector<int> frames;
    float duration;
    bool repeat;

    Animation(std::initializer_list<int> frames, float duration) {
        for (auto i = frames.begin(); i != frames.end(); i++)
            this->frames.push_back(*i);

        this->duration = duration;
        this->type = AnimationType::Timer;
        this->repeat = true;
    }


    Animation(std::initializer_list<int> frames, float duration, bool repeat) : Animation(frames, duration) {
        this->repeat = repeat;
    }

    Animation(std::vector<int> frames, float duration) {
        this->frames = frames;
        this->duration = duration;
        this->type = AnimationType::Timer;
        this->repeat = true;
    }

    Animation(std::initializer_list<int> frames) {
        for (auto i = frames.begin(); i != frames.end(); i++)
            this->frames.push_back(*i);

        this->type = AnimationType::Static;
        this->repeat = true;
    }

    Animation(std::vector<int> frames) {
        this->type = AnimationType::Static;
        this->repeat = true;
    }

    int getFrame(int frame) {
        return this->frames[frame];
    }

    unsigned int getLength() { return frames.size(); }
};

class AnimationHandler
{
private:

    /* Array of animations */
    std::vector<Animation> animations;


    int currentColumn;
    int currentFrame;

public:
    /* Current section of the texture that should be displayed */
    sf::IntRect bounds;

    /* Pixel dimensions of each individual frame */
    sf::IntRect frameSize;
    /* Current time since the animation loop started */
    float t;
    int l;
    bool newFrame;

    std::function<void(int)> changeFrameCallback;

    int count() {return animations.size();};

    int getCurrentFrame() {
        return currentFrame;
    }
    int getCurrentColumn() {
        return currentColumn;
    }
    /* Add a new animation */
    void addAnim(Animation anim) {
        this->animations.push_back(anim);
        return;
    }

    void set(int frame) {
        /* Set the sprite to the new frame */
        sf::IntRect rect = this->frameSize;
        rect.left = rect.width * this->currentColumn;
        rect.top = rect.height * frame;
        this->bounds = rect;
    }

    Animation &getAnim() {
        return this->animations[this->currentColumn];
    }

    /* Update the current frame of animation. dt is the time since
     * the update was last called (i.e. the time for one frame to be
     * executed) */
    void update(const float dt) {
        if (currentColumn < 0 || currentColumn >= this->animations.size()) return;

        Animation &anim = this->animations[currentColumn];

        if (anim.type == AnimationType::Timer) {

            this->newFrame = false;

            float duration = anim.duration;

            /* Check if the animation has progessed to a new frame and if so
             * change to the next frame */
            if (int((t + dt) / duration) > int(t / duration))
            {
                /* Calculate the frame number */
                int frame = int((t + dt) / duration);

                /* Adjust for looping */
                if (anim.repeat)
                    frame %= anim.getLength();

                if (frame != this->currentFrame) {
                    this->newFrame = true;
                    this->changeFrameCallback(frame);
                }

                this->currentFrame = frame;

                if (frame < anim.getLength()) {
                    frame = anim.getFrame(frame);

                    this->set(frame);
                }

            }

            /* Increment the time elapsed */
            this->t += dt;
            /* Adjust for looping */
            if (anim.repeat) {
                if (this->t > duration * anim.getLength())
                {
                    this->t = 0.0f;
                    this->l++;
                }
            } else {
                if (this->t > duration * anim.getLength()) {
                    this->l = 1;
                }
            }
        }

        return;
    }

    /* Change the animation, resetting t in the process */
    void changeColumn(unsigned int col) {
        /* Do not change the animation if the animation is currently active or
        * the new animation does not exist */
        if (this->currentColumn == col || col < 0 || col >= this->animations.size()) return;

        /* Set the current animation */
        this->currentColumn = col;

        /* Update the animation bounds */
        sf::IntRect rect = this->frameSize;

        rect.left = rect.width * this->currentColumn;
        rect.top = rect.height * this->currentFrame;

        this->bounds = rect;
        this->t = 0.0;
        this->l = 0;

        return;
    }

    void reset() {
        this->t = 0.0f;
        this->currentFrame = 0;
        this->currentColumn = 0;
        this->l = 0;
        this->newFrame = false;
        this->changeFrameCallback = [](int frame) {};
    }

    /* Constructor */
    AnimationHandler()
    {
        this->reset();
    }
    AnimationHandler(const sf::IntRect& frameSize)
    {
        this->frameSize = frameSize;
        this->reset();
    }
};

#endif /* ANIMATION_HANDLER_HPP */
