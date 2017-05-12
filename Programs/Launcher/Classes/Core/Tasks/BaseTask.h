#pragma once

#include <QString>
#include <QVariant>

#include <functional>

class ApplicationManager;

//task data component
class TaskDataHolder
{
public:
    void SetUserData(const QVariant& data);
    const QVariant& GetUserData() const;

private:
    QVariant userData;
};

//task operation result component
class ErrorHolder
{
public:
    //OperationProcessor member functions must work in inherited class const methods
    void SetError(const QString& error) const;
    QString GetError() const;

    bool HasError() const;

private:
    mutable QString errorText;
};

class BaseTask : public TaskDataHolder, public ErrorHolder
{
public:
    BaseTask(ApplicationManager* appManager);

    virtual ~BaseTask() = default;

    //enum to avoid dynamic casts
    enum eTaskType
    {
        RUN_TASK,
        DOWNLOAD_TASK,
        ZIP_TASK,
        ASYNC_CHAIN
    };

    virtual QString GetDescription() const = 0;
    virtual eTaskType GetTaskType() const = 0;

    using CallbackFn = std::function<void(const BaseTask*)>;
    void SetOnFinishCallback(CallbackFn callback);
    void SetOnSuccessCallback(CallbackFn callback);

protected:
    ApplicationManager* appManager = nullptr;

private:
    friend class ReceiverNotifier;

    BaseTask(const BaseTask& task) = delete;
    BaseTask(BaseTask&& task) = delete;
    BaseTask& operator=(const BaseTask& task) = delete;
    BaseTask& operator=(BaseTask&& task) = delete;

    CallbackFn onFinishedCallback;
    CallbackFn onSuccessCallback;
};

//base class for run tasks without progress bar
class RunTask : public BaseTask
{
public:
    virtual void Run() = 0;
    eTaskType GetTaskType() const override;

protected:
    RunTask(ApplicationManager* appManager);
};
