

// g++ --std=c++11 test.cpp

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest.h"
#include "fx/frame.h"
#include "fx/video/frame_interpolator.h"
#include "namespace.h"

TEST_CASE("FrameInterpolator::selectFrames") {
    SUBCASE("Empty interpolator") {
        FrameInterpolator interpolator(5);
        const Frame *selected1;
        const Frame *selected2;
        CHECK_FALSE(interpolator.selectFrames(0, &selected1, &selected2));
    }

    SUBCASE("2 frame interpolator before") {
        // Create an interpolator with capacity for 2 frames
        FrameInterpolator interpolator(2);

        // Create some test frames with different timestamps
        Frame frame1(10, false); // 10 pixels, no alpha
        Frame frame2(10, false);

        // Add frames with timestamps
        CHECK(interpolator.addWithTimestamp(frame1, 1000));
        CHECK(interpolator.addWithTimestamp(frame2, 2000));

        const Frame *selected1;
        const Frame *selected2;

        // Falls between two frames.
        bool selected = interpolator.selectFrames(0, &selected1, &selected2);
        CHECK(selected);
        CHECK(selected1);
        CHECK(selected2);
        // Now check that it's the same frame.
        CHECK(selected1 == selected2);
        // now check that the timestamp of the first frame is less than the
        // timestamp of the second frame
        CHECK(selected1->getTimestamp() == 1000);
        CHECK(selected2->getTimestamp() == 1000);
    }

    SUBCASE("2 frame interpolator between") {
        // Create an interpolator with capacity for 2 frames
        FrameInterpolator interpolator(2);

        // Create some test frames with different timestamps
        Frame frame1(10, false); // 10 pixels, no alpha
        Frame frame2(10, false);

        // Add frames with timestamps
        CHECK(interpolator.addWithTimestamp(frame1, 0));
        CHECK(interpolator.addWithTimestamp(frame2, 1000));

        const Frame *selected1;
        const Frame *selected2;

        // Falls between two frames.
        bool selected = interpolator.selectFrames(500, &selected1, &selected2);
        CHECK(selected);
        CHECK(selected1);
        CHECK(selected2);
        // now check that the frames are different
        CHECK(selected1 != selected2);
        // now check that the timestamp of the first frame is less than the
        // timestamp of the second frame
        CHECK(selected1->getTimestamp() == 0);
        CHECK(selected2->getTimestamp() == 1000);
    }

    SUBCASE("2 frame interpolator after") {
        // Create an interpolator with capacity for 2 frames
        FrameInterpolator interpolator(2);

        // Create some test frames with different timestamps
        Frame frame1(10, false); // 10 pixels, no alpha
        Frame frame2(10, false);

        // Add frames with timestamps
        CHECK(interpolator.addWithTimestamp(frame1, 0));
        CHECK(interpolator.addWithTimestamp(frame2, 1000));

        const Frame *selected1;
        const Frame *selected2;

        // Falls between two frames.
        bool selected = interpolator.selectFrames(1500, &selected1, &selected2);
        CHECK(selected);
        CHECK(selected1);
        CHECK(selected2);
        // now check that the frames are different
        CHECK(selected1 == selected2);
        // now check that the timestamp of the first frame is less than the
        // timestamp of the second frame
        CHECK(selected1->getTimestamp() == 1000);
        CHECK(selected2->getTimestamp() == 1000);
    }
}

TEST_CASE("FrameInterpolator::addWithTimestamp") {
    SUBCASE("add first frame") {
        FrameInterpolator interpolator(5);
        Frame frame(10, false);
        CHECK(interpolator.addWithTimestamp(frame, 1000));
        FrameInterpolator::FrameBuffer &frames = interpolator.getFrames();
        CHECK_EQ(frames.size(), 1);
        CHECK_EQ(frames.front()->getTimestamp(), 1000);
    }

    SUBCASE("add second frame which is before first frame and should be rejected") {
        FrameInterpolator interpolator(5);
        Frame frame1(10, false);
        Frame frame2(10, false);
        CHECK(interpolator.addWithTimestamp(frame1, 1000));
        CHECK_FALSE(interpolator.addWithTimestamp(frame2, 500));
        FrameInterpolator::FrameBuffer &frames = interpolator.getFrames();
        CHECK_EQ(frames.size(), 1);
        CHECK_EQ(frames.front()->getTimestamp(), 1000);
    }

    
    SUBCASE("add second frame which has the same timestamp as first frame and should be rejected") {
        FrameInterpolator interpolator(5);
        Frame frame1(10, false);
        Frame frame2(10, false);
        CHECK(interpolator.addWithTimestamp(frame1, 1000));
        CHECK_FALSE(interpolator.addWithTimestamp(frame2, 1000));
        FrameInterpolator::FrameBuffer &frames = interpolator.getFrames();
        CHECK_EQ(frames.size(), 1);
        CHECK_EQ(frames.front()->getTimestamp(), 1000);
    }

    SUBCASE("add second frame which is after first frame and should be accepted") {
        FrameInterpolator interpolator(5);
        Frame frame1(10, false);
        Frame frame2(10, false);
        CHECK(interpolator.addWithTimestamp(frame1, 1000));
        CHECK(interpolator.addWithTimestamp(frame2, 1500));
        FrameInterpolator::FrameBuffer &frames = interpolator.getFrames();
        CHECK_EQ(frames.size(), 2);
        CHECK_EQ(frames.front()->getTimestamp(), 1500);
        CHECK_EQ(frames.back()->getTimestamp(), 1000);
    }

}

TEST_CASE("FrameInterpolator::draw") {
    SUBCASE("Empty interpolator") {
        FrameInterpolator interpolator(5);
        Frame frame(10, false);
        Frame dst(10, false);
        CHECK_FALSE(interpolator.draw(0, &dst));
    }

    SUBCASE("Add one frame and check that we will draw with that") {
        FrameInterpolator interpolator(5);
        Frame frame(10, false);
        CHECK(interpolator.addWithTimestamp(frame, 1000));
        Frame dst(10, false);
        CHECK(interpolator.draw(0, &dst));
        CHECK_EQ(dst.getTimestamp(), 1000);
        CHECK(interpolator.draw(2000, &dst));
        CHECK_EQ(dst.getTimestamp(), 1000);
    }

    SUBCASE("Add two frames and check behavior for drawing before, between and after") {
        FrameInterpolator interpolator(5);
        Frame frame1(10, false);
        Frame frame2(10, false);
        CHECK(interpolator.addWithTimestamp(frame1, 1000));
        CHECK(interpolator.addWithTimestamp(frame2, 2000));
        Frame dst(10, false);
        CHECK(interpolator.draw(0, &dst));
        CHECK_EQ(dst.getTimestamp(), 1000);
        CHECK(interpolator.draw(1500, &dst));
        CHECK_EQ(dst.getTimestamp(), 1500);
        CHECK(interpolator.draw(2500, &dst));
        CHECK_EQ(dst.getTimestamp(), 2000);
    }
}