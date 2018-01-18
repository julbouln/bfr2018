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

    Animation(std::initializer_list<int> frames, float duration) {
        for (auto i = frames.begin(); i != frames.end(); i++)
            this->frames.push_back(*i);

        this->duration = duration;
        this->type = AnimationType::Timer;
    }


    Animation(std::vector<int> frames, float duration) {
        this->frames = frames;
        this->duration = duration;
        this->type = AnimationType::Timer;
    }

    Animation(std::initializer_list<int> frames) {
        for (auto i = frames.begin(); i != frames.end(); i++)
            this->frames.push_back(*i);

        this->type = AnimationType::Static;
    }

    Animation(std::vector<int> frames) {
        this->type = AnimationType::Static;
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

    /* Current time since the animation loop started */
    float t;

    int currentAnim;
    int currentFrame;

public:
    int l;

    int count() {return animations.size();};

    int getCurrentFrame() {
        return currentFrame;
    }
    int getCurrentAnim() {
        return currentAnim;
    }
    /* Add a new animation */
    void addAnim(Animation anim) {
        this->animations.push_back(anim);
        return;
    }

    void set(int frame) {
        this->currentFrame = frame;
        /* Set the sprite to the new frame */
        sf::IntRect rect = this->frameSize;
        rect.left = rect.width * this->currentAnim;
        rect.top = rect.height * frame;
        this->bounds = rect;
    }

    /* Update the current frame of animation. dt is the time since
     * the update was last called (i.e. the time for one frame to be
     * executed) */
    void update(const float dt) {
        if (currentAnim >= this->animations.size() || currentAnim < 0) return;

        if (this->animations[currentAnim].type == AnimationType::Timer) {

            float duration = this->animations[currentAnim].duration;

            /* Check if the animation has progessed to a new frame and if so
             * change to the next frame */
            if (int((t + dt) / duration) > int(t / duration))
            {
                /* Calculate the frame number */
                int frame = int((t + dt) / duration);

                /* Adjust for looping */
                frame %= this->animations[currentAnim].getLength();
                frame = this->animations[currentAnim].getFrame(frame);

                this->set(frame);
            }

            /* Increment the time elapsed */
            this->t += dt;
            /* Adjust for looping */
            if (this->t > duration * this->animations[currentAnim].getLength())
            {
                this->t = 0.0f;
                this->l++;
            }
        }

        return;
    }

    /* Change the animation, resetting t in the process */
    void changeAnim(unsigned int animID) {
        /* Do not change the animation if the animation is currently active or
        * the new animation does not exist */
        if (this->currentAnim == animID || animID >= this->animations.size() ||
                animID < 0) return;

        /* Set the current animation */
        this->currentAnim = animID;
        /* Update the animation bounds */
        sf::IntRect rect = this->frameSize;

        rect.left = rect.width * animID;
        rect.top = rect.height * currentFrame;

        this->bounds = rect;
        this->t = 0.0;
        this->l = 0;

        return;
    }

    /* Current section of the texture that should be displayed */
    sf::IntRect bounds;

    /* Pixel dimensions of each individual frame */
    sf::IntRect frameSize;

    /* Constructor */
    AnimationHandler()
    {
        this->t = 0.0f;
        this->currentFrame = 0;
        this->currentAnim = -1;
        this->l = 0;
    }
    AnimationHandler(const sf::IntRect& frameSize)
    {
        this->frameSize = frameSize;

        this->t = 0.0f;
        this->currentFrame = 0;
        this->currentAnim = -1;
        this->l = 0;
    }
};

#endif /* ANIMATION_HANDLER_HPP */
