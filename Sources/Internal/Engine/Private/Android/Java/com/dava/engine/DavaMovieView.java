package com.dava.engine;

// Duplicates enum eMovieScalingMode declared in UI/IMovieViewControl.h
class eMovieScalingMode
{
    static final int scalingModeNone = 0;
    static final int scalingModeAspectFit = 1;
    static final int scalingModeAspectFill = 2;
    static final int scalingModeFill = 3;
};

final class DavaMovieView
{
    // Duplicates enum MovieViewControl::eAction declared in UI/Private/Android/MovieViewControlAndroid.h
    class eAction
    {
        static final int ACTION_PLAY = 0;
        static final int ACTION_PAUSE = 1;
        static final int ACTION_RESUME = 2;
        static final int ACTION_STOP = 3;
    }

    // About java volatile https://docs.oracle.com/javase/tutorial/essential/concurrency/atomic.html
    private volatile long movieviewBackendPointer = 0;
    private DavaSurfaceView surfaceView = null;

    // Properties that have been set in DAVA::Engine thread and waiting to apply to MovieView
    private MovieViewProperties properties = new MovieViewProperties();

    // Some properties that reflect MovieView current properties
    float x;
    float y;
    float width;
    float height;

    public static native void nativeReleaseWeakPtr(long backendPointer);

    private class MovieViewProperties
    {
        MovieViewProperties() {}
        MovieViewProperties(MovieViewProperties other)
        {
            x = other.x;
            y = other.y;
            width = other.width;
            height = other.height;
            visible = other.visible;
            moviePath = other.moviePath;
            scaleMode = other.scaleMode;
            action = other.action;

            createNew = other.createNew;
            anyPropertyChanged = other.anyPropertyChanged;
            rectChanged = other.rectChanged;
            visibleChanged = other.visibleChanged;
            movieChanged = other.movieChanged;
            actionChanged = other.actionChanged;
        }

        float x;
        float y;
        float width;
        float height;
        boolean visible;
        String moviePath;
        int scaleMode;
        int action;

        boolean createNew;
        boolean anyPropertyChanged;
        boolean rectChanged;
        boolean visibleChanged;
        boolean movieChanged;
        boolean actionChanged;

        void clearChangedFlags()
        {
            createNew = false;
            anyPropertyChanged = false;
            rectChanged = false;
            visibleChanged = false;
            movieChanged = false;
            actionChanged = false;
        }
    }

    public DavaMovieView(DavaSurfaceView view, long backendPointer)
    {
        movieviewBackendPointer = backendPointer;
        surfaceView = view;

        properties.createNew = true;
        properties.anyPropertyChanged = true;
    }

    void release()
    {
        DavaActivity.commandHandler().post(new Runnable() {
            @Override public void run()
            {
                releaseNativeControl();
            }
        });
    }

    void setRect(float x, float y, float width, float height)
    {
        boolean changed = properties.x != x || properties.y != y ||
                          properties.width != width || properties.height != height; 
        if (changed)
        {
            properties.x = x;
            properties.y = y;
            properties.width = width;
            properties.height = height;
            properties.rectChanged = true;
            properties.anyPropertyChanged = true;
        }
    }

    void setVisible(boolean visible)
    {
        if (properties.visible != visible)
        {
            properties.visible = visible;
            properties.visibleChanged = true;
            properties.anyPropertyChanged = true;
            if (!visible)
            {
                // Immediately hide native control
                DavaActivity.commandHandler().post(new Runnable() {
                    @Override public void run()
                    {
                        /*if (nativeWebView != null)
                        {
                            setNativeVisible(false);
                        }*/
                    }
                });
            }
        }
    }

    void openMovie(String moviePath, int scaleMode)
    {
        properties.moviePath = moviePath;
        properties.scaleMode = scaleMode;
        properties.movieChanged = true;
        properties.anyPropertyChanged = true;
    }

    void doAction(int action)
    {
        properties.action = action;
        properties.actionChanged = true;
        properties.anyPropertyChanged = true;
    }

    void update()
    {
        if (properties.anyPropertyChanged)
        {
            final MovieViewProperties props = new MovieViewProperties(properties);
            DavaActivity.commandHandler().post(new Runnable() {
                @Override public void run()
                {
                    processProperties(props);
                }
            });
            properties.clearChangedFlags();
        }
    }

    void processProperties(MovieViewProperties props)
    {
        if (props.createNew)
        {
            createNativeControl();
        }
        if (props.anyPropertyChanged)
        {
            applyChangedProperties(props);
        }
    }

    void createNativeControl()
    {
    }

    void releaseNativeControl()
    {
        nativeReleaseWeakPtr(movieviewBackendPointer);
        movieviewBackendPointer = 0;
    }

    void applyChangedProperties(MovieViewProperties props)
    {
        /*if (props.movieChanged)
            ;
        if (props.actionChanged)
            applyAction(props.action);
        if (props.rectChanged || props.visibleChanged)
        {
            x = props.x;
            y = props.y;
            width = props.width;
            height = props.height;
            setNativeVisible(props.visible);
            setNativePositionAndSize(x, y, width, height);
        }*/
    }

    void setNativePositionAndSize(float x, float y, float width, float height)
    {
        //surfaceView.positionControl(nativeWebView, x, y, width, height);
    }

    void setNativeVisible(boolean visible)
    {

    }

    void applyAction(int action)
    {

    }
}
