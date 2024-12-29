/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CreatureScript.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include "SpellScriptLoader.h"
#include "halls_of_stone.h"

#define GOSSIP_ITEM_1       "布莱恩，能帮助你是我们的荣幸！"
#define GOSSIP_ITEM_2       "走吧布莱恩,别再给我们上历史课了!"
#define GOSSIP_ITEM_3       "我们现在没时间管这个,我们必须继续前进."
#define GOSSIP_ITEM_4       "我们和你在一起布莱恩！打开它！"
#define TEXT_ID_START       13100

enum NPCs
{
    NPC_DARK_RUNE_PROTECTOR         = 27983,
    NPC_DARK_RUNE_STORMCALLER       = 27984,
    NPC_IRON_GOLEM_CUSTODIAN        = 27985,
    NPC_DARK_MATTER                 = 28235,
    NPC_DARK_MATTER_TARGET          = 28237,
    NPC_SEARING_GAZE_TRIGGER        = 28265,
};

enum Misc
{
    // BRANN EVENT
    SPELL_GLARE_OF_THE_TRIBUNAL     = 50988,
    SPELL_GLARE_OF_THE_TRIBUNAL_H   = 59870,
    SPELL_DARK_MATTER_VISUAL        = 51000,
    SPELL_DARK_MATTER_VISUAL_CHANNEL= 51001,
    SPELL_DARK_MATTER               = 51012,
    SPELL_DARK_MATTER_H             = 59868,
    SPELL_SEARING_GAZE              = 51136,
    SPELL_SEARING_GAZE_H            = 59867,

    // DARK RUNE PROTECTOR
    SPELL_DRP_CHARGE                = 22120,
    SPELL_DRP_CLEAVE                = 42724,

    // DARK RUNE STORMCALLER
    SPELL_DRS_LIGHTING_BOLT         = 12167,
    SPELL_DRS_LIGHTING_BOLT_H       = 59863,
    SPELL_DRS_SHADOW_WORD_PAIN      = 15654,
    SPELL_DRS_SHADOW_WORD_PAIN_H    = 59864,

    // IRON GOLEM CUSTODIAN
    SPELL_IGC_CRUSH_ARMOR           = 33661,
    SPELL_IGC_GROUND_SMASH          = 12734,
    SPELL_IGC_GROUND_SMASH_H        = 59865,

    // QUESTS
    QUEST_HALLS_OF_STONE            = 13207,
};

enum events
{
    // BRANN AND TRIBUNAL
    EVENT_ABEDNEUM_VISUAL = 1,
    EVENT_KADDRAK_VISUAL = 2,
    EVENT_MARNAK_VISUAL = 3,
    EVENT_ABEDNEUM_HEAD = 4,
    EVENT_KADDRAK_HEAD = 5,
    EVENT_MARNAK_HEAD = 6,
    EVENT_KADDRAK_SWITCH_EYE = 7,
    EVENT_SUMMON_MONSTERS = 8,
    EVENT_SUMMON_STORMCALLER = 9,
    EVENT_SUMMON_CUSTODIAN = 10,
    EVENT_DARK_MATTER_START = 11,
    EVENT_DARK_MATTER_END = 12,

    // DARK RUNE PROTECTOR
    EVENT_DRP_CHARGE = 13,
    EVENT_DRP_CLEAVE = 14,

    // DARK RUNE STORMCALLER
    EVENT_DRS_LIGHTNING_BOLD = 15,
    EVENT_DRS_SHADOW_WORD_PAIN = 16,

    // IRON GOLEM CUSTODIAN
    EVENT_IGC_CRUSH = 17,
    EVENT_IGC_GROUND_SMASH = 18,

    EVENT_TRIBUNAL_END = 19,
    EVENT_BREEN_WAITING = 20,
    EVENT_TALK_FACE_CHANGE = 21,
    EVENT_SKY_ROOM_FLOOR_CHANGE = 22,

    //BRANN AND SJONNIR
    EVENT_GO_TO_SJONNIR = 23,
    EVENT_DOOR_OPEN = 24,
    EVENT_RESUME_ESCORT = 25,
    EVENT_SJONNIR_END_BRANN_YELL = 26,
    EVENT_SJONNIR_END_BRANN_LAST_YELL = 27,
};

struct Yells
{
    uint32 sound;
    const char* text;
    uint32 creature, timer;
};

static Yells Conversation[] =
{
    {14259, "是该收获答案了!让我们一路杀过去吧!", NPC_BRANN, 0},
    {14247, "休息一会，然后和我一起来启动这个东西。一切……马上就要真相大白了。好了，我们开始吧！", NPC_BRANN, 5000},
    {14248, "嗯，你们帮我看着点外面。我这样的强者只要锤两下就能搞定这破烂……", NPC_BRANN, 17000},
    {13765, "警告：生命体形态模式无法辨识。文档处理程序中断。继续干涉程序将导致安全对策启动。", NPC_ABEDNEUM, 20500},
    {14249, "噢，见鬼，好像有点不太妙，看来我们遇到点小麻烦啦……", NPC_BRANN, 32000},
    {13756, "安全系统发现不明入侵。历史文档的分析工作优先级转为低。对策程序立即启动。", NPC_KADDRAK, 37000},
    {14250, "嘿，破机器，怎么样？你想跟我来硬的？太可惜了，这是属于我的游戏！", NPC_BRANN, 49000},
    {14251, "再给我几分钟，马上就能……啊噢……", NPC_BRANN, 100000},
    {13761, "已超出威胁指数标准。天界文档中断。提高安全级别。", NPC_MARNAK, 105000},
    {14252, "提高？这算是好消息吗？", NPC_BRANN, 116000},
    {14253, "哈！终于找到问题出在哪儿了！我马上就能搞定了……", NPC_BRANN, 195000},
    {13767, "威胁指数过高。虚空分析程序关闭。启动清理协议。", NPC_ABEDNEUM, 205000},
    {14254, "等等！你想清理谁啊！有我在，没门！", NPC_BRANN, 215000},
    {14255, "哈！看来还是我这把老骨头厉害呀！然后再看看这里……", NPC_BRANN, 295000},
    {13768, "警告：安全系统自动修复装置已被关闭。立刻消除化全部存储器内容并……", NPC_ABEDNEUM, 303000},
    //The fight is completed at this point.
    {14256, "消除？不不不不……呀，我刚才搞到哪来着？啊哈！应该是这样！", NPC_BRANN, 310000},
    {13769, "系统已上线。生命形态模式已确认。您好，铜须先生。是否需要问询？", NPC_ABEDNEUM, 321000},
    {14263, "问询？要不你以为我来这干嘛？喝下午茶吗？少废话！", NPC_BRANN, 329000},
    {14264, "告诉我矮人是怎样演化出来的！从最初开始！", NPC_BRANN, 336000},
    {13770, "正在检索史前数据……文档定位完毕。创世之初，土灵的使命是--", NPC_ABEDNEUM, 342000},
    {14265, "对，对。我已经知道土灵是用石头创造出来，用以改造地下世界的。但变异是怎么发生的？我听说过什么母体扰动？", NPC_BRANN, 348000},
    {13771, "正在检索……在其发展周期的初始阶段，艾泽拉斯世界遭受到了寄生性噬主共体生物的感染。", NPC_ABEDNEUM, 360000},
    {14266, "寄生性噬主……什么来着？讲通用语会死吗？", NPC_BRANN, 373500},
    {13772, "名称：上古之神。上古之神感染了包括土灵在内的整个毫无防备的系统，意图将所有事物向他们自身同化。这种扰动母体结构的行为与现象被定义为“血肉诅咒”。扰动的效果与日俱增。", NPC_ABEDNEUM, 380000},
    {14267, "上古之神是吧，也就是说他们给土灵施加了什么血肉的诅咒，然后怎么样了？", NPC_BRANN, 399500},
    {13757, "正在检索……造物主曾经亲自回来清除噬主共体生物所造成的感染。根据当时的评估，上古之神滋生所造成的危害非常严重。强行分离被感染部分将导致宿主损毁--", NPC_KADDRAK, 406000},
    {14268, "如果他们真的杀光上古之神，那艾泽拉斯也就毁灭了……", NPC_BRANN, 424000},
    {13758, "正确。造物主决定中和寄生体的威胁，并将其封锁在宿主内部。之后，包括意志熔炉在内的各系统受命开始创造新的土灵。植入了新的安全机制，并任命了守护者。", NPC_KADDRAK, 429000},
    {14269, "什么守护者？", NPC_BRANN, 449000},
    {13759, "名称：艾塞尔与瓦尼尔。如果用通用语命名，则是风暴巨人与岩石巨人，最高领袖为哨兵洛肯。守护巨龙受命监管艾泽拉斯世界的进化过程。", NPC_KADDRAK, 452000},
    {14270, "艾塞尔与瓦尼尔……好吧，这么说意志熔炉又从头开始创造了新的土灵……那老的怎么样了？", NPC_BRANN, 471000},
    {13762, "另有后台信息与你的问询关联：在艾塞尔和瓦尼尔的全面战争之后--", NPC_MARNAK, 482000},
    {14271, "等等！等等！你是说艾塞尔与瓦尼尔之间爆发了战争？为什么？", NPC_BRANN, 489000},
    {13763, "不详。数据显示，全面战争是由首席管理官洛肯发起的，他在此之后中和了所有残余的艾塞尔与瓦尼尔，结束了冲突。接着，他停止了包括土灵、巨人和维库人在内的数个源种生物的演化进程，并将他们全部封存在指定的封存仓内。", NPC_MARNAK, 494000},
    {14272, "这个“洛肯”听起来真是阴险狡诈。还好不用再理会他了。那么……如果我的理解没有错，最初的原始土灵是否因为某种原因从休眠仓里苏醒了过来，还受了这个什么母体扰动的影响，最终演化成现代的矮人？至少……是现代矮人的祖先。", NPC_BRANN, 519000},
    {13764, "本质上的确如此。", NPC_MARNAK, 543000},
    {14273, "啊，现在我得花上很长时间来好好消化这些知识。我要在这里再待一会儿。谢谢你们。", NPC_BRANN, 549000},
    {13773, "收到，布莱恩布隆赞。进程终止。", NPC_ABEDNEUM, 559000},
    {0, "让我们看看入口附近的大门后面藏着什么。我会悄悄地潜过去。去那里和我碰面吧，我会带你们进去吧。", NPC_BRANN, 574000},
};

class brann_bronzebeard : public CreatureScript
{
public:
    brann_bronzebeard() : CreatureScript("brann_bronzebeard") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        InstanceScript* pInstance = (creature->GetInstanceScript());

        player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
        player->PrepareGossipMenu(creature, 0, true);
        if (pInstance)
        {
            uint32 brann = pInstance->GetData(BRANN_BRONZEBEARD);
            switch (brann)
            {
                case 1:
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_ITEM_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                    SendGossipMenuFor(player, TEXT_ID_START, creature->GetGUID());
                    break;
                case 2:
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_ITEM_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                    SendGossipMenuFor(player, TEXT_ID_TRIBUNAL_START, creature->GetGUID());
                    break;
                case 3:
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_ITEM_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                    SendGossipMenuFor(player, TEXT_ID_TRIBUNAL_END, creature->GetGUID());
                    break;
                case 4:
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_ITEM_4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                    SendGossipMenuFor(player, TEXT_ID_SJONNIR_DOOR, creature->GetGUID());
                    break;
                case 5:
                    SendGossipMenuFor(player, TEXT_ID_SJONNIR_END, creature->GetGUID());
                    break;
                default:
                    break;
            }
        }
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32  /*sender*/, uint32 action) override
    {
        if (action)
        {
            switch (action)
            {
                case GOSSIP_ACTION_INFO_DEF+1:
                    creature->AI()->DoAction(ACTION_START_ESCORT_EVENT);
                    CloseGossipMenuFor(player);
                    break;
                case GOSSIP_ACTION_INFO_DEF+2:
                    creature->AI()->DoAction(ACTION_START_TRIBUNAL);
                    CloseGossipMenuFor(player);
                    break;
                case GOSSIP_ACTION_INFO_DEF+3:
                    creature->AI()->DoAction(ACTION_GO_TO_SJONNIR);
                    CloseGossipMenuFor(player);
                    break;
                case GOSSIP_ACTION_INFO_DEF+4:
                    creature->AI()->DoAction(ACTION_OPEN_DOOR);
                    CloseGossipMenuFor(player);
                    break;
            }
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new brann_bronzebeardAI (creature);
    }

    struct brann_bronzebeardAI : public npc_escortAI
    {
        brann_bronzebeardAI(Creature* c) : npc_escortAI(c), summons(me)
        {
            pInstance = c->GetInstanceScript();
        }

        InstanceScript* pInstance;
        EventMap events;
        SummonList summons;
        ObjectGuid AbedneumGUID;
        ObjectGuid MarnakGUID;
        ObjectGuid KaddrakGUID;
        ObjectGuid darkMatterTargetGUID;

        uint8 WaveNum;

        bool TalkEvent;
        uint32 SpeechCount, SpeechPause;
        bool canExecuteEvents = true;

        void DespawnHeads()
        {
            Creature* cr;
            if ((cr = GetAbedneum())) cr->DespawnOrUnsummon();
            if ((cr = GetMarnak())) cr->DespawnOrUnsummon();
            if ((cr = GetKaddrak())) cr->DespawnOrUnsummon();

            SwitchHeadVisaul(0x7, false);
        }

        void SwitchHeadVisaul(uint8 headMask, bool activate)
        {
            if (!pInstance)
                return;

            GameObject* go = nullptr;
            if (headMask & 0x1) // Kaddrak
                if ((go = me->GetMap()->GetGameObject(pInstance->GetGuidData(GO_KADDRAK))))
                {
                    if (activate)
                    {
                        go->SendCustomAnim(0);
                    }
                    else
                    {
                        go->SendCustomAnim(1);
                        if (go->GetGoState() == GO_STATE_ACTIVE)
                            go->SetGoState(GO_STATE_READY);
                    }
                }
            if (headMask & 0x2) // Marnak
                if ((go = me->GetMap()->GetGameObject(pInstance->GetGuidData(GO_MARNAK))))
                {
                    if (activate)
                    {
                        go->SendCustomAnim(0);
                    }
                    else
                    {
                        go->SendCustomAnim(1);
                        if (go->GetGoState() == GO_STATE_ACTIVE)
                            go->SetGoState(GO_STATE_READY);
                    }
                }
            if (headMask & 0x4) // Abedneum
                if ((go = me->GetMap()->GetGameObject(pInstance->GetGuidData(GO_ABEDNEUM))))
                {
                    if (activate)
                    {
                        go->SendCustomAnim(0);
                    }
                    else
                    {
                        go->SendCustomAnim(1);
                        if (go->GetGoState() == GO_STATE_ACTIVE)
                            go->SetGoState(GO_STATE_READY);
                    }
                }
        }

        void ResetEvent()
        {
            if (GameObject* tribunal = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(GO_TRIBUNAL_CONSOLE)))
                tribunal->SetGoState(GO_STATE_READY);

            if (GameObject* tribunalSkyFloor = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(GO_SKY_FLOOR)))
                tribunalSkyFloor->SetGoState(GO_STATE_READY);

            events.Reset();
            summons.DespawnAll();
            DespawnHeads();

            WaveNum = 0;
            SpeechCount = 0;
            SpeechPause = 0;
            TalkEvent = false;
        }

        void WaypointReached(uint32 id) override;
        void InitializeEvent();

        Creature* GetAbedneum() { return ObjectAccessor::GetCreature(*me, AbedneumGUID); }
        Creature* GetMarnak() { return ObjectAccessor::GetCreature(*me, MarnakGUID); }
        Creature* GetKaddrak() { return ObjectAccessor::GetCreature(*me, KaddrakGUID); }

        bool leftEye = true;

        void DamageTaken(Unit*, uint32& damage, DamageEffectType, SpellSchoolMask) override
        {
            if (damage && pInstance)
                pInstance->SetData(DATA_BRANN_ACHIEVEMENT, false);
        }

        void Reset() override
        {
            RemoveEscortState(0x7); // all states
            SetDespawnAtFar(false);
            SetDespawnAtEnd(false);
            ResetEvent();

            me->SetFaction(FACTION_FRIENDLY);
            me->SetReactState(REACT_PASSIVE);
            me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);

            if (pInstance)
            {
                pInstance->SetData(BRANN_BRONZEBEARD, 1);
                pInstance->SetData(DATA_BRANN_ACHIEVEMENT, true);

                if (pInstance->GetData(BOSS_TRIBUNAL_OF_AGES) == DONE)
                {
                    pInstance->SetData(BRANN_BRONZEBEARD, 4);
                    if (GameObject* door = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(GO_TRIBUNAL_ACCESS_DOOR)))
                        door->SetGoState(GO_STATE_ACTIVE);
                }

                if (pInstance->GetData(BOSS_SJONNIR) == DONE)
                {
                    pInstance->SetData(BRANN_BRONZEBEARD, 5);
                    if (GameObject* door = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(GO_SJONNIR_DOOR)))
                        door->SetGoState(GO_STATE_ACTIVE);
                }
            }
        }

        void DoAction(int32 action) override
        {
            switch (action)
            {
                case ACTION_START_ESCORT_EVENT:
                    Start(false, true, ObjectGuid::Empty, 0, true, false);
                    Talk(SAY_BRANN_ESCORT_START);
                    me->SetFaction(FACTION_ESCORTEE_N_NEUTRAL_PASSIVE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->SetRegeneratingHealth(false);
                    break;
                case ACTION_START_TRIBUNAL:
                {
                    me->SetReactState(REACT_PASSIVE);
                    Map::PlayerList const& PlayerList = me->GetMap()->GetPlayers();
                    if (!PlayerList.IsEmpty())
                        for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                        {
                            me->SetFaction(i->GetSource()->GetFaction());
                            break;
                        }

                    SetEscortPaused(false);
                    InitializeEvent();
                    me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_NONE);
                    break;
                }
                case ACTION_TRIBUNAL_WIPE_START:
                    SetNextWaypoint(1, false);
                    SetEscortPaused(false);
                    ResetEvent();
                    me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_NONE);
                    break;
                case ACTION_GO_TO_SJONNIR:
                    Talk(SAY_BRANN_ENTRANCE_MEET);
                    me->SetFaction(FACTION_FRIENDLY);
                    me->SetReactState(REACT_PASSIVE);
                    me->SetRegeneratingHealth(true);
                    SetEscortPaused(false);
                    ResetEvent();
                    me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_NONE);
                    DoCast(me, 58506, false);
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY_UNARMED);
                    me->SendMovementFlagUpdate();
                    break;
                case ACTION_START_SJONNIR_FIGHT:
                    me->SetFaction(FACTION_FRIENDLY);
                    me->Yell("嘿！别担心！老布莱恩在后面给你们加油呢！把这些该死的金属疙瘩给我挡住，让我来好好跟这台可爱的大机器谈谈，让它帮帮你们！", LANG_UNIVERSAL);
                    me->PlayDirectSound(14274);
                    SetEscortPaused(false);
                    break;
                case ACTION_SJONNIR_DEAD:
                    me->Yell("洛肯？！这下我们碰到大麻烦了……虽然铁矮人已经被全部摆平了，但我敢打赌不知道什么鬼地方还有台机器，正不停向外吐着铁维库人！", LANG_UNIVERSAL);
                    me->PlayDirectSound(14278);
                    events.ScheduleEvent(EVENT_END, 14000);
                    SetEscortPaused(false);
                    break;
                case ACTION_SJONNIR_DEAD:
                    if (pInstance)
                        pInstance->SetData(BRANN_BRONZEBEARD, 5);
                    SetEscortPaused(false);
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                    me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                    me->SetOrientation(3.132660f);
                    me->SendMovementFlagUpdate();
                    events.ScheduleEvent(EVENT_SJONNIR_END_BRANN_YELL, 10000ms);
                    events.ScheduleEvent(EVENT_SJONNIR_END_BRANN_LAST_YELL, 22000ms);
                    break;
                case ACTION_SJONNIR_WIPE_START:
                    Reset();
                    SetNextWaypoint(33, true);
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY_UNARMED);
                    me->StopMovingOnCurrentPos();
                    DoCast(me, 58506, false);
                    if (GameObject* door = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(GO_SJONNIR_DOOR)))
                        door->SetGoState(GO_STATE_READY);
                    break;
                case ACTION_OPEN_DOOR:
                    Start(false, true, ObjectGuid::Empty, 0, true, false);
                    SetNextWaypoint(34, false);
                    SetEscortPaused(false);
                    me->RemoveAura(58506);
                    me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_NONE);
                    me->SetWalk(true);
                    me->SetSpeed(MOVE_WALK, 1.0f);
                    break;
            }
        }

        void JustSummoned(Creature* cr) override
        {
            if (cr->GetEntry() == NPC_ABEDNEUM || cr->GetEntry() == NPC_KADDRAK || cr->GetEntry() == NPC_MARNAK)
                cr->SetCanFly(true);
            else
                summons.Summon(cr);
        }

        void UpdateEscortAI(uint32 diff) override
        {
            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_KADDRAK_VISUAL:
                    {
                        SwitchHeadVisaul(0x1, true);
                        break;
                    }
                    case EVENT_MARNAK_VISUAL:
                    {
                        SwitchHeadVisaul(0x2, true);
                        break;
                    }
                    case EVENT_ABEDNEUM_VISUAL:
                    {
                        SwitchHeadVisaul(0x4, true);
                        break;
                    }
                    case EVENT_KADDRAK_HEAD: // Phase 1
                    {
                        if (!canExecuteEvents)
                            return;
                        if (Creature* kaddrak = GetKaddrak())
                        {
                            if (Player* plr = SelectTargetFromPlayerList(100.0f))
                                kaddrak->CastSpell(plr, DUNGEON_MODE(SPELL_GLARE_OF_THE_TRIBUNAL, SPELL_GLARE_OF_THE_TRIBUNAL_H), true);
                        }

                        events.RescheduleEvent(EVENT_KADDRAK_SWITCH_EYE, 1000ms);
                        events.Repeat(1500ms);
                        break;
                    }
                    case EVENT_KADDRAK_SWITCH_EYE:
                    {
                        if (!canExecuteEvents)
                            return;
                        if (Creature* kaddrak = GetKaddrak())
                        {
                            if (leftEye)
                                kaddrak->UpdatePosition(927.9f, 330.9f, 219.4f, 2.4f, true);
                            else
                                kaddrak->UpdatePosition(923.7f, 326.9f, 219.5f, 2.1f, true);

                            leftEye = !leftEye;
                            kaddrak->StopMovingOnCurrentPos();
                        }

                        break;
                    }
                    case EVENT_MARNAK_HEAD: // Phase 2
                    {
                        if (!canExecuteEvents)
                            return;

                        if (Creature* marnak = GetMarnak())
                        {
                            if (Creature* cr = me->SummonCreature(NPC_DARK_MATTER_TARGET, 899.843f, 355.271f, 214.301f, 0, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            {
                                cr->SetCanFly(true);

                                //right eye
                                if (Creature* cra = me->SummonCreature(NPC_DARK_MATTER, marnak->GetPositionX(), marnak->GetPositionY(), marnak->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 5000))
                                    cra->CastSpell(cra, SPELL_DARK_MATTER_VISUAL_CHANNEL, false);

                                //left eye
                                if (Creature* crb = me->SummonCreature(NPC_DARK_MATTER, 891.543f, 359.5252f, 219.338f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 5000))
                                    crb->CastSpell(crb, SPELL_DARK_MATTER_VISUAL_CHANNEL, false);

                                darkMatterTargetGUID = cr->GetGUID();

                                events.RescheduleEvent(EVENT_DARK_MATTER_START, 5000ms);
                            }
                        }
                        events.Repeat(30s);
                        break;
                    }
                    case EVENT_DARK_MATTER_START:
                    {
                        if (Creature* darkMatterTarget = ObjectAccessor::GetCreature(*me, darkMatterTargetGUID))
                        {
                            darkMatterTarget->CastSpell(darkMatterTarget, SPELL_DARK_MATTER_VISUAL, false);
                            if (Player* plr = SelectTargetFromPlayerList(100.0f))
                            {
                                if (!plr)
                                    return; //no target

                                float speed = 10.0f;
                                float tooFarAwaySpeed = me->GetDistance(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ()) / (5000.0f * 0.001f);
                                if (speed < tooFarAwaySpeed)
                                    speed = tooFarAwaySpeed;

                                darkMatterTarget->MonsterMoveWithSpeed(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), speed);

                                if (darkMatterTarget->GetDistance(plr) < 15.0f)
                                {
                                    events.RescheduleEvent(EVENT_DARK_MATTER_END, 3000ms);
                                }
                                else if (darkMatterTarget->GetDistance(plr) < 30.0f)
                                {
                                    events.RescheduleEvent(EVENT_DARK_MATTER_END, 3500ms);
                                }
                                else
                                {
                                    events.RescheduleEvent(EVENT_DARK_MATTER_END, 4500ms);
                                }
                            }
                        }
                        break;
                    }
                    case EVENT_DARK_MATTER_END:
                    {
                        if (Creature* darkMatterTarget = ObjectAccessor::GetCreature(*me, darkMatterTargetGUID))
                        {
                            darkMatterTarget->CastSpell(darkMatterTarget, darkMatterTarget->GetMap()->IsHeroic() ? SPELL_DARK_MATTER_H : SPELL_DARK_MATTER, true);
                            darkMatterTarget->DespawnOrUnsummon(500);
                        }
                        break;
                    }
                    case EVENT_ABEDNEUM_HEAD: // Phase 3
                    {
                        if (!canExecuteEvents)
                            return;
                        if (GetAbedneum())
                        {
                            Player* plr = SelectTargetFromPlayerList(100.0f);
                            if (!plr)
                                break;

                            if (Creature* cr = me->SummonCreature(NPC_SEARING_GAZE_TRIGGER, plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ(), 0, TEMPSUMMON_TIMED_DESPAWN, 10000))
                            {
                                // summon another abedneum to create double beam, despawn just after trigger despawn
                                me->SummonCreature(NPC_ABEDNEUM, 897.0f, 326.9f, 223.5f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 12000);
                                cr->CastSpell(cr, DUNGEON_MODE(SPELL_SEARING_GAZE, SPELL_SEARING_GAZE_H), true);
                            }
                        }
                        events.Repeat(15s);
                        break;
                    }
                    case EVENT_SUMMON_MONSTERS:
                    {
                        if (!canExecuteEvents)
                            return;
                        uint32 Time = 40000 - (2500 * WaveNum);
                        SummonCreatures(NPC_DARK_RUNE_PROTECTOR, 3, 0);
                        if (WaveNum > 2)
                            events.ScheduleEvent(EVENT_SUMMON_STORMCALLER, urand(10 - WaveNum, 15 - WaveNum) * 1000);
                        if (WaveNum > 5)
                            events.ScheduleEvent(EVENT_SUMMON_CUSTODIAN, urand(10 - WaveNum, 15 - WaveNum) * 1000);

                        WaveNum++;
                        events.RepeatEvent(Time);
                        break;
                    }
                    case EVENT_SUMMON_STORMCALLER:
                    {
                        if (!canExecuteEvents)
                            return;

                        SummonCreatures(NPC_DARK_RUNE_STORMCALLER, 2, 1);

                        break;
                    }
                    case EVENT_SUMMON_CUSTODIAN:
                    {
                        if (!canExecuteEvents)
                            return;

                        SummonCreatures(NPC_IRON_GOLEM_CUSTODIAN, 1, 1);

                        break;
                    }
                    case EVENT_TRIBUNAL_END:
                    {
                        canExecuteEvents = false;
                        // Has to be here!
                        events.Reset();
                        //DespawnHeads();
                        summons.DespawnAll();

                        // Spawn Chest and quest credit
                        if (Player* plr = SelectTargetFromPlayerList(200.0f))
                        {
                            if (GameObject* go = plr->SummonGameObject((IsHeroic() ? GO_TRIBUNAL_CHEST_H : GO_TRIBUNAL_CHEST), 880.406f, 345.164f, 203.706f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0))
                            {
                                plr->RemoveGameObject(go, false);
                                go->SetLootMode(1);
                                go->ReplaceAllGameObjectFlags((GameObjectFlags)0);
                            }

                            plr->GroupEventHappens(QUEST_HALLS_OF_STONE, me);
                        }

                        events.ScheduleEvent(EVENT_BREEN_WAITING, 11s);
                        events.ScheduleEvent(EVENT_SKY_ROOM_FLOOR_CHANGE, 92s);
                        events.ScheduleEvent(EVENT_TALK_FACE_CHANGE, 97s);          //kaddrak speaks
                        events.ScheduleEvent(EVENT_SKY_ROOM_FLOOR_CHANGE, 168s);
                        events.ScheduleEvent(EVENT_TALK_FACE_CHANGE, 173s);         //marnak speaks
                        events.ScheduleEvent(EVENT_SKY_ROOM_FLOOR_CHANGE, 239s);
                        events.ScheduleEvent(EVENT_TALK_FACE_CHANGE, 244s);         //abedneum speaks
                        events.ScheduleEvent(EVENT_GO_TO_SJONNIR, 251s);
                        events.ScheduleEvent(EVENT_SKY_ROOM_FLOOR_CHANGE, 253s);
                        break;
                    }
                    case EVENT_BREEN_WAITING:
                    {
                        SetEscortPaused(false);
                        if (pInstance)
                        {
                            pInstance->SetData(BOSS_TRIBUNAL_OF_AGES, DONE);
                            pInstance->SetData(BRANN_BRONZEBEARD, 3);
                            me->CastSpell(me, 59046, true); // credit
                        }

                        me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                        me->Yell("我会用熔炉来制造一些土灵，让他们保卫这里……但现在最大的挑战是：找到并阻止洛肯！", LANG_UNIVERSAL);
                        me->PlayDirectSound(14279);
                        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                        me->SendMovementFlagUpdate();

                        break;
                    }
                    case EVENT_TALK_FACE_CHANGE:
                    {
                        if (pInstance)
                            pInstance->SetData(BOSS_TRIBUNAL_OF_AGES, DONE);
                        break;
                    }
                    case EVENT_SKY_ROOM_FLOOR_CHANGE:
                    {
                        if (pInstance)
                            pInstance->SetData(BOSS_TRIBUNAL_OF_AGES, SPECIAL);
                        break;
                    }
                    case EVENT_GO_TO_SJONNIR:
                    {
                        me->AI()->DoAction(ACTION_GO_TO_SJONNIR);
                        break;
                    }
                    case EVENT_DOOR_OPEN:
                    {
                        me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_NONE);
                        me->AddUnitMovementFlag(MOVEMENTFLAG_NONE);

                        if (pInstance)
                        {
                            if (GameObject* door = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(GO_SJONNIR_DOOR)))
                            {
                                door->SetGoState(GO_STATE_ACTIVE);
                                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY_UNARMED);
                                me->SendMovementFlagUpdate();
                            }
                        }

                        break;
                    }
                    case EVENT_RESUME_ESCORT:
                    {
                        SetEscortPaused(false);
                        break;
                    }
                    case EVENT_SJONNIR_END_BRANN_YELL:
                    {
                        Talk(SAY_BRANN_VICTORY_SJONNIR_1);
                        break;
                    }
                    case EVENT_SJONNIR_END_BRANN_LAST_YELL:
                    {
                        events.Reset();
                        SetEscortPaused(false);
                        Talk(SAY_BRANN_VICTORY_SJONNIR_2);
                        break;
                    }
                }
            }

            npc_escortAI::UpdateEscortAI(diff);

            if (TalkEvent)
            {
                SpeechPause += diff;
                if (SpeechPause >= Conversation[SpeechCount].timer)
                {
                    Creature* cs = nullptr;
                    switch (Conversation[SpeechCount].creature)
                    {
                        case NPC_BRANN:
                            cs = me;
                            break;
                        case NPC_ABEDNEUM:
                            cs = GetAbedneum();
                            break;
                        case NPC_KADDRAK:
                            cs = GetKaddrak();
                            break;
                        case NPC_MARNAK:
                            cs = GetMarnak();
                            break;
                    }

                    if (cs)
                    {
                        cs->Yell(Conversation[SpeechCount].text, LANG_UNIVERSAL);
                        cs->PlayDirectSound(Conversation[SpeechCount].sound);
                    }

                    if (SpeechCount < 38)
                        SpeechPause = Conversation[SpeechCount++].timer;
                    else
                        TalkEvent = false;
                }
            }
        }

        void SummonCreatures(uint32 entry, uint8 count, uint8 pos)
        {
            Creature* cr;
            for (int i = 0; i < count; ++i)
            {
                if (pos == 0)
                    cr = me->SummonCreature(entry, 943.088f + urand(0, 5), 401.378f + urand(0, 5), 206.078f, 3.8f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);     //left
                else
                    cr = me->SummonCreature(entry, 964.302f + urand(0, 4), 378.942f + urand(0, 4), 206.078f, 3.85f, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);    //right

                if (cr)
                {
                    cr->AI()->AttackStart(me);
                    cr->AddThreat(me, 100.0f);
                    cr->SetInCombatWithZone();
                }
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            ResetEvent();
            if (pInstance)
            {
                if (Creature* brann = ObjectAccessor::GetCreature(*me, pInstance->GetGuidData(NPC_BRANN)))
                {
                    brann->HandleEmoteCommand(EMOTE_STATE_DEAD);
                    brann->setDeathState(DeathState::JustDied);
                    brann->Respawn();
                    if (pInstance->GetData(BOSS_TRIBUNAL_OF_AGES) != DONE)
                    {
                        brann->AI()->DoAction(ACTION_TRIBUNAL_WIPE_START);
                        pInstance->SetData(BOSS_TRIBUNAL_OF_AGES, FAIL);
                    }
                    if (pInstance->GetData(BOSS_TRIBUNAL_OF_AGES) == DONE)
                    {
                        brann->AI()->DoAction(ACTION_SJONNIR_WIPE_START);
                    }
                }
                if (pInstance->GetData(BOSS_TRIBUNAL_OF_AGES) != DONE)
                    pInstance->SetData(BOSS_TRIBUNAL_OF_AGES, NOT_STARTED);
            }
        }
    };
};

void brann_bronzebeard::brann_bronzebeardAI::InitializeEvent()
{
    Creature* cr = nullptr;
    if ((cr = me->SummonCreature(NPC_KADDRAK, 923.7f, 326.9f, 219.5f, 2.1f, TEMPSUMMON_TIMED_DESPAWN, 580000)))
    {
        cr->SetInCombatWithZone();
        KaddrakGUID = cr->GetGUID();
    }
    if ((cr = me->SummonCreature(NPC_MARNAK, 895.974f, 363.571f, 219.337f, 5.5f, TEMPSUMMON_TIMED_DESPAWN, 580000)))
    {
        cr->SetInCombatWithZone();
        MarnakGUID = cr->GetGUID();
    }
    if ((cr = me->SummonCreature(NPC_ABEDNEUM, 892.25f, 331.25f, 223.86f, 0.6f, TEMPSUMMON_TIMED_DESPAWN, 580000)))
    {
        cr->SetInCombatWithZone();
        AbedneumGUID = cr->GetGUID();
    }

    TalkEvent = true;

    events.Reset();

    // Viusals
    events.ScheduleEvent(EVENT_KADDRAK_VISUAL, 30s);
    events.ScheduleEvent(EVENT_MARNAK_VISUAL, 105s);
    events.ScheduleEvent(EVENT_ABEDNEUM_VISUAL, 207s);

    // Fight
    events.ScheduleEvent(EVENT_SUMMON_MONSTERS, 47s);
    events.ScheduleEvent(EVENT_KADDRAK_HEAD, 47s);
    events.ScheduleEvent(EVENT_MARNAK_HEAD, 115s);
    events.ScheduleEvent(EVENT_ABEDNEUM_HEAD, 217s);
    events.ScheduleEvent(EVENT_TRIBUNAL_END, 310s);
}

void brann_bronzebeard::brann_bronzebeardAI::WaypointReached(uint32 id)
{
    switch (id)
    {
        // Stop before stairs and ask to start
        case 14:
            SetEscortPaused(true);
            if (pInstance)
            {
                me->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);
                Talk(SAY_BRANN_EVENT_INTRO_1);
                me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                pInstance->SetData(BRANN_BRONZEBEARD, 2);
            }
            break;
        // In front of Console
        case 16:
            SetEscortPaused(true);
            if (pInstance)
            {
                pInstance->SetData(BOSS_TRIBUNAL_OF_AGES, IN_PROGRESS);
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_USE_STANDING);
                if (GameObject* tribunal = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(GO_TRIBUNAL_CONSOLE)))
                    tribunal->SetGoState(GO_STATE_ACTIVE);
            }
            break;
        //Tribunal end, stand in the middle of the sky room
        case 17:
            SetEscortPaused(true);
            me->SetOrientation(3.91672f);
            me->SendMovementFlagUpdate();
            break;
        //Run to the skyroom door and then teleport before Sjonnir's door
        case 18:
            SetEscortPaused(true);
            SetNextWaypoint(33, false);
            if (pInstance)
            {
                pInstance->SetData(BRANN_BRONZEBEARD, 4);
                me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                if (Creature* cr = ObjectAccessor::GetCreature(*me, pInstance->GetGuidData(NPC_SJONNIR)))
                    cr->RemoveUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
                me->SetOrientation(3.132660f);
                DoCast(me, 58506, false);
                me->SendMovementFlagUpdate();
                me->SetHomePosition(1199.8f, 667.138f, 196.242f, 3.12967f);
                me->Relocate(1199.8f, 667.138f, 196.242f, 3.12967f);
            }
            break;
        // Before Sjonnir's door
        case 33:
            SetEscortPaused(true);
            break;
        //Walk to the door, run after opening it
        case 34:
            SetEscortPaused(true);
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_USE_STANDING);
            me->SendMovementFlagUpdate();
            events.ScheduleEvent(EVENT_DOOR_OPEN, 1500);
            me->SetWalk(false);
            me->SetSpeed(MOVE_RUN, 1.0f, false);
            events.ScheduleEvent(EVENT_RESUME_ESCORT, 3500);
            break;
        //Brann stops in front of Sjonnir and awaits the start of the battle.
        case 36:
            SetEscortPaused(true);
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY_UNARMED);
            me->SendMovementFlagUpdate();
            Talk(SAY_BRANN_FRONT_OF_SJONNIR);
            break;
        //Brann steps back and uses the Sjonnir console.
        case 38:
            SetEscortPaused(true);
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_USE_STANDING);
            if (pInstance)
            {
                if (GameObject* console = ObjectAccessor::GetGameObject(*me, pInstance->GetGuidData(GO_SJONNIR_CONSOLE)))
                    console->SetGoState(GO_STATE_ACTIVE);
            }
            break;
        //After Sjonnir's death, Brann steps away from the console and talk.
        case 39:
            SetEscortPaused(true);
            break;
        //Brann steps back and uses the Sjonnir console.
        case 40:
            SetEscortPaused(true);
            me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_USE_STANDING);
            break;
    }
}

class dark_rune_protectors : public CreatureScript
{
public:
    dark_rune_protectors() : CreatureScript("dark_rune_protectors") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new dark_rune_protectorsAI (creature);
    }

    struct dark_rune_protectorsAI : public ScriptedAI
    {
        dark_rune_protectorsAI(Creature* c) : ScriptedAI(c) { }

        EventMap events;
        void Reset() override
        {
            events.Reset();
        }

        void JustEngagedWith(Unit*) override
        {
            events.ScheduleEvent(EVENT_DRP_CHARGE, 10s);
            events.ScheduleEvent(EVENT_DRP_CLEAVE, 7s);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EVENT_DRP_CHARGE:
                    {
                        if (Unit* tgt = SelectTarget(SelectTargetMethod::Random, 0))
                            me->CastSpell(tgt, SPELL_DRP_CHARGE, false);

                        events.Repeat(10s);
                        break;
                    }
                case EVENT_DRP_CLEAVE:
                    {
                        me->CastSpell(me->GetVictim(), SPELL_DRP_CLEAVE, false);
                        events.Repeat(7s);
                        break;
                    }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class dark_rune_stormcaller : public CreatureScript
{
public:
    dark_rune_stormcaller() : CreatureScript("dark_rune_stormcaller") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new dark_rune_stormcallerAI (creature);
    }

    struct dark_rune_stormcallerAI : public ScriptedAI
    {
        dark_rune_stormcallerAI(Creature* c) : ScriptedAI(c) { }

        EventMap events;
        void Reset() override
        {
            events.Reset();
        }

        void JustEngagedWith(Unit*) override
        {
            events.ScheduleEvent(EVENT_DRS_LIGHTNING_BOLD, 5s);
            events.ScheduleEvent(EVENT_DRS_SHADOW_WORD_PAIN, 12s);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EVENT_DRS_LIGHTNING_BOLD:
                    {
                        me->CastSpell(me->GetVictim(), IsHeroic() ? SPELL_DRS_LIGHTING_BOLT_H : SPELL_DRS_LIGHTING_BOLT, false);
                        events.Repeat(5s);
                        break;
                    }
                case EVENT_DRS_SHADOW_WORD_PAIN:
                    {
                        me->CastSpell(me->GetVictim(), IsHeroic() ? SPELL_DRS_SHADOW_WORD_PAIN_H : SPELL_DRS_SHADOW_WORD_PAIN, false);
                        events.Repeat(12s);
                        break;
                    }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class iron_golem_custodian : public CreatureScript
{
public:
    iron_golem_custodian() : CreatureScript("iron_golem_custodian") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new iron_golem_custodianAI (creature);
    }

    struct iron_golem_custodianAI : public ScriptedAI
    {
        iron_golem_custodianAI(Creature* c) : ScriptedAI(c) { }
        EventMap events;
        void Reset() override
        {
            events.Reset();
        }

        void JustEngagedWith(Unit*) override
        {
            events.ScheduleEvent(EVENT_IGC_CRUSH, 6s);
            events.ScheduleEvent(EVENT_IGC_GROUND_SMASH, 4s);
        }
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            switch (events.ExecuteEvent())
            {
                case EVENT_IGC_CRUSH:
                    {
                        me->CastSpell(me->GetVictim(), SPELL_IGC_CRUSH_ARMOR, false);
                        events.Repeat(6s);
                        break;
                    }
                case EVENT_IGC_GROUND_SMASH:
                    {
                        me->CastSpell(me->GetVictim(), IsHeroic() ? SPELL_IGC_GROUND_SMASH_H : SPELL_IGC_GROUND_SMASH, false);
                        events.Repeat(5s);
                        break;
                    }
            }

            DoMeleeAttackIfReady();
        }
    };
};

class spell_hos_dark_matter : public AuraScript
{
    PrepareAuraScript(spell_hos_dark_matter);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_DARK_MATTER_H, SPELL_DARK_MATTER });
    }

    void HandleEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
            caster->CastSpell(caster, caster->GetMap()->IsHeroic() ? SPELL_DARK_MATTER_H : SPELL_DARK_MATTER, true);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_hos_dark_matter::HandleEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_hos_dark_matter_size : public SpellScript
{
    PrepareSpellScript(spell_hos_dark_matter_size);

    void HandleApplyTouch()
    {
        if (Unit* target = GetHitUnit())
            target->SetObjectScale(0.35f);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_hos_dark_matter_size::HandleApplyTouch);
    }
};

void AddSC_brann_bronzebeard()
{
    new brann_bronzebeard();
    new dark_rune_protectors();
    new dark_rune_stormcaller();
    new iron_golem_custodian();
    RegisterSpellScript(spell_hos_dark_matter);
    RegisterSpellScript(spell_hos_dark_matter_size);
}
