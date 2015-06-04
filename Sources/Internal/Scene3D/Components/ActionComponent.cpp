/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Scene3D/Entity.h"
#include "Particles/ParticleEmitter.h"
#include "Sound/SoundEvent.h"
#include "Scene3D/Components/ActionComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/SoundComponent.h"
#include "Scene3D/Components/WaveComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Systems/ActionUpdateSystem.h"

#include "Utils/Random.h"


namespace DAVA
{

    void ActionComponent::Action::actualizeDelay()
    {
        actualDelay = static_cast<float32>( delay + Random::Instance()->RandFloat(delayVariation) );
    }

    const FastName ActionComponent::ACTION_COMPONENT_SELF_ENTITY_NAME("*** Self ***");

	ActionComponent::ActionComponent()
        : started(false)
        , allActionsActive(false)
	{
	}
	
	ActionComponent::~ActionComponent()
	{
		if(entity &&
		   entity->GetScene())
		{
			entity->GetScene()->actionSystem->UnWatch(this);
		}
	}
	
	ActionComponent::Action ActionComponent::MakeAction(ActionComponent::Action::eType type, const FastName& targetName, float32 delay)
	{
		Action action;
		
		action.type = type;
		action.entityName = targetName;
		action.delay = delay;
		
		return action;
	}

	ActionComponent::Action ActionComponent::MakeAction(ActionComponent::Action::eType type, const FastName& targetName, float32 delay, int32 switchIndex)
	{
		Action action;
		
		action.type = type;
		action.entityName = targetName;
		action.delay = delay;
		action.switchIndex = switchIndex;
		
		return action;
	}
	
	void ActionComponent::StartSwitch(int32 switchIndex)
	{
		if (entity->GetScene()->actionSystem->IsBlockEvent(Action::EVENT_SWITCH_CHANGED))
			return;

        StopSwitch(switchIndex);
        uint32 markedCount = 0;
        uint32 count = static_cast<uint32>(actions.size());
        for ( uint32 i = 0; i < count; ++i )
        {
            Action& action = actions[i].action;
            if (( action.eventType == Action::EVENT_SWITCH_CHANGED ) && ( action.switchIndex == switchIndex ))
            {
                action.actualizeDelay();
                actions[i].markedForUpdate = true;
                markedCount++;
            }
        }

        if ( markedCount > 0 )
        {
            if ( !started )
            {
                entity->GetScene()->actionSystem->Watch( this );
            }

            started = true;
            allActionsActive = false;
        }
	}

	void ActionComponent::StartAdd()
	{		
        if (entity->GetScene()->actionSystem->IsBlockEvent(Action::EVENT_ADDED_TO_SCENE))
            return;

        uint32 markedCount = 0;
        uint32 count = static_cast<uint32>(actions.size());
        for ( uint32 i = 0; i < count; ++i )
        {
            Action& action = actions[i].action;
            if (action.eventType == Action::EVENT_ADDED_TO_SCENE)
            {
                action.actualizeDelay();
                actions[i].markedForUpdate = true;
                markedCount++;
            }
        }

        if (markedCount > 0)
        {
            if (!started)
            {
                entity->GetScene()->actionSystem->Watch(this);
            }

            started = true;
            allActionsActive = false;
        }
    }

    void ActionComponent::StartUser(const FastName& name)
    {
        if (entity->GetScene()->actionSystem->IsBlockEvent(Action::EVENT_CUSTOM))
            return;

        StopUser(name);
        uint32 markedCount = 0;
        uint32 count = static_cast<uint32>(actions.size());
        for (uint32 i = 0; i < count; ++i)
        {
            Action& action = actions[i].action;
            if (( action.eventType == Action::EVENT_CUSTOM ) && ( action.userEventId == name ))
            {
                action.actualizeDelay();
                actions[i].markedForUpdate = true;
                markedCount++;
            }
        }

        if (markedCount > 0)
        {
            if (!started)
            {
                entity->GetScene()->actionSystem->Watch(this);
            }

            started = true;
            allActionsActive = false;
        }
    }
	
	bool ActionComponent::IsStarted()
	{
		return started;
	}
	
	void ActionComponent::StopAll()
	{
		if(started)
		{
			started = false;
			allActionsActive = false;
			
			entity->GetScene()->actionSystem->UnWatch(this);
		}
		
		uint32 count = static_cast<uint32>(actions.size());
		for(uint32 i = 0; i < count; ++i)
		{
			actions[i].active = false;
			actions[i].timer = 0.0f;
			actions[i].markedForUpdate = false;
		}
	}
	
	void ActionComponent::StopSwitch(int32 switchIndex)
	{
		uint32 markedCount = 0;
		uint32 count = static_cast<uint32>(actions.size());
		for(uint32 i = 0; i < count; ++i)
		{
            Action& action = actions[i].action;
			if((action.eventType == Action::EVENT_SWITCH_CHANGED) && (action.switchIndex == switchIndex))
			{
				actions[i].active = false;
				actions[i].timer = 0.0f;
				actions[i].markedForUpdate = false;
			}
			
			if(actions[i].markedForUpdate)
			{
				markedCount++;
			}
		}

		if(started &&
		   0 == markedCount)
		{			
			started = false;
			allActionsActive = false;
				
			entity->GetScene()->actionSystem->UnWatch(this);
		}
	}

    void ActionComponent::StopUser(const FastName& name)
    {
        uint32 markedCount = 0;
        uint32 count = static_cast<uint32>(actions.size());
        for ( uint32 i = 0; i < count; ++i )
        {
            Action& action = actions[i].action;
            if (( action.eventType == Action::EVENT_CUSTOM ) && ( action.userEventId == name ))
            {
                actions[i].active = false;
                actions[i].timer = 0.0f;
                actions[i].markedForUpdate = false;
            }

            if (actions[i].markedForUpdate)
            {
                markedCount++;
            }
        }

        if ( started &&
            0 == markedCount )
        {
            started = false;
            allActionsActive = false;

            entity->GetScene()->actionSystem->UnWatch(this);
        }
    }
	
	void ActionComponent::Add(const ActionComponent::Action& action)
	{
		actions.push_back(ActionContainer(action));
		allActionsActive = false;
	}
	
	void ActionComponent::Remove(const ActionComponent::Action::eType type, const FastName& entityName, const int switchIndex)
	{
        bool wasMarked = false;
		for(auto it = actions.begin(); it < actions.end(); ++it)
		{
			const Action& innerAction = (*it).action;
			if(innerAction.type == type &&
			   innerAction.entityName == entityName &&
			   innerAction.switchIndex == switchIndex)
			{
                wasMarked = (*it).markedForUpdate;
				actions.erase(it);
				break;
			}
		}
		
		uint32 activeActionCount = 0;
        uint32 markedCount = 0;
		uint32 count = static_cast<uint32>(actions.size());
		for(uint32 i = 0; i < count; ++i)
		{
			if(actions[i].active)
			{
				activeActionCount++;
			}
            if (actions[i].markedForUpdate)
            {
                markedCount++;
            }
		}
		
		bool prevActionsActive = allActionsActive;
		allActionsActive = (activeActionCount == count);
		
        //we should un-watch in to cases
        //a. if after removing it appears that all actions are already added
        //b. if after removing marked action it appears we have no marked actions anymore
		if ((!prevActionsActive && (allActionsActive != prevActionsActive)) || (wasMarked && !markedCount))        
		{
			entity->GetScene()->actionSystem->UnWatch(this);
		}
	}
	
	void ActionComponent::Remove(const ActionComponent::Action& action)
	{
		Remove(action.type, action.entityName, action.switchIndex);
	}
	
	uint32 ActionComponent::GetCount()
	{
		return static_cast<uint32>(actions.size());
	}
	
	ActionComponent::Action& ActionComponent::Get(uint32 index)
	{
		DVASSERT(index < actions.size());
		return actions[index].action;
	}
	
	void ActionComponent::Update(float32 timeElapsed)
	{
		//do not evaluate time if all actions started
		if(started && !allActionsActive)
		{
			uint32 activeActionCount = 0;
			uint32 count = static_cast<uint32>(actions.size());
			for(uint32 i = 0; i < count; ++i)
			{
				ActionContainer& container = actions[i];
				if(!container.active &&
				   container.markedForUpdate)
				{
					container.timer += timeElapsed;
					
                    if ( container.timer >= container.action.actualDelay )
					{
						container.active = true;
						EvaluateAction(container.action);
					}
				}
				
				if(container.active)
				{
					activeActionCount++;
				}
			}
			
			bool prevActionsActive = allActionsActive;
			allActionsActive = (activeActionCount == count);
			
			if(!prevActionsActive &&
			   allActionsActive != prevActionsActive)
			{
				started = false;
				entity->GetScene()->actionSystem->UnWatch(this);
			}
		}
	}
	
	Component * ActionComponent::Clone(Entity * toEntity)
	{
		ActionComponent* actionComponent = new ActionComponent();
		actionComponent->SetEntity(toEntity);
		
		uint32 count = static_cast<uint32>(actions.size());
		actionComponent->actions.resize(count);
		for(uint32 i = 0; i < count; ++i)
		{
			actionComponent->actions[i] = actions[i];
			actionComponent->actions[i].active = false;
            actionComponent->actions[i].markedForUpdate = false;
			actionComponent->actions[i].timer = 0.0f;
		}
		
		return actionComponent;
	}
	
	void ActionComponent::Serialize(KeyedArchive *archive, SerializationContext *serializationContext)
	{
		Component::Serialize(archive, serializationContext);
		
		if(NULL != archive)
		{
			uint32 count = static_cast<uint32>(actions.size());
			archive->SetUInt32("ac.actionCount", count);
			
			for(uint32 i = 0; i < count; ++i)
			{
				KeyedArchive* actionArchive = new KeyedArchive();
                const Action& action = actions[i].action;

				actionArchive->SetUInt32("act.event", action.eventType);
				actionArchive->SetFloat("act.delay", action.delay);
                actionArchive->SetFloat( "act.delayVariation", action.delayVariation );
				actionArchive->SetUInt32("act.type", action.type);
                actionArchive->SetFastName("act.userEventId", action.userEventId);
				actionArchive->SetString("act.entityName", String(action.entityName.c_str() ? action.entityName.c_str() : ""));
				actionArchive->SetInt32("act.switchIndex", action.switchIndex);
				actionArchive->SetInt32("act.stopAfterNRepeats", action.stopAfterNRepeats);
				actionArchive->SetBool("act.stopWhenEmpty", action.stopWhenEmpty);
			
				archive->SetArchive(KeyedArchive::GenKeyFromIndex(i), actionArchive);
				SafeRelease(actionArchive);
			}			
		}
	}
	
	void ActionComponent::Deserialize(KeyedArchive *archive, SerializationContext *serializationContext)
	{
		actions.clear();
		
		if(NULL != archive)
		{
			uint32 count = archive->GetUInt32("ac.actionCount");
			for(uint32 i = 0; i < count; ++i)
			{
				KeyedArchive* actionArchive = archive->GetArchive(KeyedArchive::GenKeyFromIndex(i));
				
				Action action;
				action.eventType = (Action::eEvent)actionArchive->GetUInt32("act.event");
				action.type = (Action::eType)actionArchive->GetUInt32("act.type");
                action.userEventId = actionArchive->GetFastName("act.userEventId", FastName(""));
				action.delay = actionArchive->GetFloat("act.delay");
                action.delayVariation = actionArchive->GetFloat( "act.delayVariation" );
				action.entityName = FastName(actionArchive->GetString("act.entityName").c_str());
				action.switchIndex = actionArchive->GetInt32("act.switchIndex", -1);
				action.stopAfterNRepeats = actionArchive->GetInt32("act.stopAfterNRepeats", -1);
				action.stopWhenEmpty = actionArchive->GetBool("act.stopWhenEmpty", false);
				
				Add(action);
			}
		}
		
		Component::Deserialize(archive, serializationContext);
	}

    void ActionComponent::EvaluateAction(const Action& action)
	{
        switch ( action.type )
        {
        case Action::TYPE_PARTICLE_EFFECT_START:
            OnActionParticleEffectStart( action );
            break;
        case Action::TYPE_PARTICLE_EFFECT_STOP:
            OnActionParticleEffectStop( action );
            break;
        case Action::TYPE_ANIMATION_START:
            OnActionAnimationStart( action );
            break;
        case Action::TYPE_ANIMATION_STOP:
            OnActionAnimationStop( action );
            break;
        case Action::TYPE_SOUND_START:
            OnActionSoundStart( action );
            break;
        case Action::TYPE_SOUND_STOP:
            OnActionSoundStop( action );
            break;
        case Action::TYPE_WAVE:
            OnActionWave( action );
            break;
        default:
            DVASSERT( false );
            break;
        }
	}
	
	void ActionComponent::OnActionParticleEffectStart(const Action& action)
	{
		Entity* target = GetTargetEntity(action.entityName, entity);
		
		if(target != NULL)
		{
			ParticleEffectComponent* component = static_cast<ParticleEffectComponent*>(target->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
			if(component)
			{
				component->StopAfterNRepeats(action.stopAfterNRepeats);
				component->StopWhenEmpty(action.stopWhenEmpty);
				component->Stop();
				component->Start();
			}
		}
	}

    void ActionComponent::OnActionParticleEffectStop( const Action& action )
    {
        Entity* target = GetTargetEntity( action.entityName, entity );

        if ( target != NULL )
        {
            ParticleEffectComponent* component = static_cast<ParticleEffectComponent*>( target->GetComponent( Component::PARTICLE_EFFECT_COMPONENT ) );
            if ( component )
            {
                component->Stop(!action.stopWhenEmpty);
            }
        }
    }

    void ActionComponent::OnActionAnimationStart(const Action& action)
    {
        Entity* target = GetTargetEntity( action.entityName, entity );
        if ( target != NULL )
        {
            AnimationComponent* component = GetAnimationComponent(target);
            if ( component )
            {
                component->StopAfterNRepeats(action.stopAfterNRepeats);
                component->Stop();
                component->Start();
            }
        }
    }

    void ActionComponent::OnActionAnimationStop(const Action& action)
    {
        Entity* target = GetTargetEntity( action.entityName, entity );
        if ( target != NULL )
        {
            AnimationComponent* component = GetAnimationComponent(target);
            if ( component )
            {
                component->StopAfterNRepeats(action.stopAfterNRepeats);
                component->Stop();
            }
        }
    }

	void ActionComponent::OnActionSoundStart(const Action& action)
	{
		Entity* target = GetTargetEntity(action.entityName, entity);
		
		if(target != NULL)
		{
			SoundComponent* component = static_cast<SoundComponent*>(target->GetComponent(Component::SOUND_COMPONENT));
			if(component)
			{
                component->Trigger();
			}

		}
	}

    void ActionComponent::OnActionSoundStop(const Action& action)
    {
        Entity* target = GetTargetEntity(action.entityName, entity);

        if(target != NULL)
        {
            SoundComponent* component = static_cast<SoundComponent*>(target->GetComponent(Component::SOUND_COMPONENT));
            if(component)
            {
                component->Stop();
            }

        }
    }

    void ActionComponent::OnActionWave(const Action& action)
    {
        Entity* target = GetTargetEntity(action.entityName, entity);

        if(target != NULL)
        {
            WaveComponent* component = GetWaveComponent(target);
            if(component)
            {
                component->Trigger();
            }

        }
    }

	Entity* ActionComponent::GetTargetEntity(const FastName& name, Entity* parent)
	{
        if(name == ACTION_COMPONENT_SELF_ENTITY_NAME)
            return parent;

		if(parent->GetName() == name)
		{
			return parent;
		}
		
		return parent->FindByName(name);		
	}

};
