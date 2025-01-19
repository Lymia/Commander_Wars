var Constructor = function()
{
    this.getCOStyles = function()
    {
        return ["+alt"];
    };

    this.init = function(co, map)
    {
        co.setPowerStars(3);
        co.setSuperpowerStars(3);
    };

    this.loadCOMusic = function(co, map)
    {
        if (CO.isActive(co))
        {
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Power:
                audio.addMusic("resources/music/cos/power.ogg", 992, 45321);
                break;
            case GameEnums.PowerMode_Superpower:
                audio.addMusic("resources/music/cos/superpower.ogg", 1505, 49515);
                break;
            case GameEnums.PowerMode_Tagpower:
                audio.addMusic("resources/music/cos/tagpower.ogg", 14611, 65538);
                break;
            default:
                audio.addMusic("resources/music/cos/rachel.ogg", 2220, 63662);
                break;
            }
        }
    };

    this.activatePower = function(co, map)
    {
        var dialogAnimation = co.createPowerSentence();
        var powerNameAnimation = co.createPowerScreen(GameEnums.PowerMode_Power);
        dialogAnimation.queueAnimation(powerNameAnimation);

        var units = co.getOwner().getUnits();
        var animations = [];
        var counter = 0;
        units.randomize();
        for (var i = 0; i < units.size(); i++)
        {
            var unit = units.at(i);
            var animation = GameAnimationFactory.createAnimation(map, unit.getX(), unit.getY());
            var delay = globals.randInt(135, 265);
            if (animations.length < 5)
            {
                delay *= i;
            }
            if (i % 2 === 0)
            {
                animation.setSound("power7_1.wav", 1, delay);
            }
            else
            {
                animation.setSound("power7_2.wav", 1, delay);
            }
            if (animations.length < 5)
            {
                animation.addSprite("power7", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                powerNameAnimation.queueAnimation(animation);
                animations.push(animation);
            }
            else
            {
                animation.addSprite("power7", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                animations[counter].queueAnimation(animation);
                animations[counter] = animation;
                counter++;
                if (counter >= animations.length)
                {
                    counter = 0;
                }
            }
        }
    };

    this.activateSuperpower = function(co, powerMode, map)
    {
        var dialogAnimation = co.createPowerSentence();
        var powerNameAnimation = co.createPowerScreen(powerMode);
        powerNameAnimation.queueAnimationBefore(dialogAnimation);

        var ret = CO_RACHEL.throwRocket(co, CO_RACHEL.superPowerDamage, GameEnums.RocketTarget_HpLowMoney, powerNameAnimation, 0, map);
        ret = CO_RACHEL.throwRocket(co, CO_RACHEL.superPowerDamage, GameEnums.RocketTarget_HpHighMoney, ret, 1, map);
        CO_RACHEL.throwRocket(co, CO_RACHEL.superPowerDamage, GameEnums.RocketTarget_Money, ret, 2, map);
    };

    this.throwRocket = function(co, damage, targetType, animation2, index, map)
    {
        var rocketTarget = co.getOwner().getRockettarget(2, damage, 1.2, targetType);
        
        var animation = GameAnimationFactory.createAnimation(map, rocketTarget.x - 2, rocketTarget.y - 2 - 1);
        animation.addSprite("explosion+silo", -map.getImageSize() / 2, 0, 0, 2, 0);
        animation.setSound("missle_explosion.wav", 1);
        animation.setEndOfAnimationCall("CO_RACHEL", "postAnimationThrowRocket" + index.toString());
        animation.setStartOfAnimationCall("CO_RACHEL", "preAnimationThrowRocket" + index.toString());
        animation2.queueAnimation(animation);
        CO_RACHEL.postAnimationThrowRocketTarget[index] = rocketTarget;
        CO_RACHEL.postAnimationThrowRocketDamage[index] = damage;
        return animation;
    };
    this.postAnimationThrowRocketTarget = [null, null, null];
    this.postAnimationThrowRocketDamage = [0, 0, 0];

    this.preAnimationThrowRocket0 = function(animation, map)
    {
        map.centerMap(CO_RACHEL.postAnimationThrowRocketTarget[0].x,
                      CO_RACHEL.postAnimationThrowRocketTarget[0].y);
    };
    this.preAnimationThrowRocket1 = function(animation, map)
    {
        map.centerMap(CO_RACHEL.postAnimationThrowRocketTarget[1].x,
                      CO_RACHEL.postAnimationThrowRocketTarget[1].y);
    };
    this.preAnimationThrowRocket2 = function(animation, map)
    {
        map.centerMap(CO_RACHEL.postAnimationThrowRocketTarget[2].x,
                      CO_RACHEL.postAnimationThrowRocketTarget[2].y);
    };

    this.postAnimationThrowRocket0 = function(animation, map)
    {
        CO_RACHEL.postAnimationThrowRocket(0, map);
    };
    this.postAnimationThrowRocket1 = function(animation, map)
    {
        CO_RACHEL.postAnimationThrowRocket(1, map);
    };
    this.postAnimationThrowRocket2 = function(animation, map)
    {
        CO_RACHEL.postAnimationThrowRocket(2, map);
    };
    this.postAnimationThrowRocket = function(index, map)
    {
        var rocketTarget = CO_RACHEL.postAnimationThrowRocketTarget[index];
        var damage = CO_RACHEL.postAnimationThrowRocketDamage[index];
        var fields = globals.getCircle(0, 2);
        // check all fields we can attack
        for (var i = 0; i < fields.size(); i++)
        {
            var x = fields.at(i).x + rocketTarget.x;
            var y = fields.at(i).y + rocketTarget.y;
            if (map.onMap(x, y))
            {
                var unit = map.getTerrain(x, y).getUnit();
                if (unit !== null)
                {
                    var hp = unit.getHpRounded();
                    if (hp - damage <= 0.1)
                    {
                        // set hp to very very low
                        unit.setHp(0.1);
                    }
                    else
                    {
                        unit.setHp(hp - damage);
                    }
                }
            }
        }
        CO_RACHEL.postAnimationThrowRocketTarget[index] = null;
        CO_RACHEL.postAnimationThrowRocketDamage[index] = 0;
    };

    this.getCOUnitRange = function(co, map)
    {
        return 2;
    };
    this.getCOArmy = function()
    {
        return "OS";
    };

    this.superPowerDamage = 3;
    this.powerLuckDamage = 40;
    this.powerOffBonus = 20;
    this.powerDefBonus = 20;
    this.d2dCoZoneOffBonus = 20;
    this.d2dCoZoneDefBonus = 20;
    this.d2dRepairBonus = 1;

    this.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                 defender, defPosX, defPosY, isDefender, action, luckmode, map)
    {
        if (CO.isActive(co))
        {
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
            case GameEnums.PowerMode_Power:
                return CO_RACHEL.powerOffBonus;
            default:
                if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                {
                    return CO_RACHEL.d2dCoZoneOffBonus;
                }
                break;
            }
        }
        return 0;
    };
    this.getDeffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                       defender, defPosX, defPosY, isAttacker, action, luckmode, map)
    {
        if (CO.isActive(co))
        {
            if (co.getPowerMode() > GameEnums.PowerMode_Off)
            {
                return CO_RACHEL.powerDefBonus;
            }
            else if (co.inCORange(Qt.point(defPosX, defPosY), defender))
            {
                return CO_RACHEL.d2dCoZoneDefBonus;
            }
        }
        return 0;
    };
    this.getBonusLuck = function(co, unit, posX, posY, map)
    {
        if (CO.isActive(co))
        {
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                break;
            case GameEnums.PowerMode_Power:
                return CO_RACHEL.powerLuckDamage;
            default:
                break;
            }
        }
        return 0;
    };

    this.getRepairBonus = function(co, unit, posX, posY, map)
    {
        if (CO.isActive(co))
        {
            if (map === null ||
                (map !== null && map.getGameRules().getCoGlobalD2D()))
            {
                return CO_RACHEL.d2dRepairBonus;
            }
        }
        return 0;
    };
    this.getAiCoUnitBonus = function(co, unit, map)
    {
        return 1;
    };
    // CO - Intel
    this.getBio = function(co)
    {
        return qsTr("Nicknamed 'The Rocket Girl', she brings a breath of fresh air to her troops. Strives to follow in the footsteps of her older sister, Nell. Led the Allied Nations during the Omega war.");
    };
    this.getHits = function(co)
    {
        return qsTr("Hard work");
    };
    this.getMiss = function(co)
    {
        return qsTr("Excuses");
    };
    this.getCODescription = function(co)
    {
        return qsTr("Her troops are quite hardworking, and they increase repairs by one on properties.");
    };
    this.getLongCODescription = function(co, map)
    {
        var values = [0];
        if (map === null ||
            (map !== null && map.getGameRules().getCoGlobalD2D()))
        {
            values = [CO_RACHEL.d2dRepairBonus];
        }
        var text = qsTr("\nGlobal Effect: \nRachel's units repair an additional +%0 HP per day on properties.") +
               qsTr("\n\nCO Zone Effect: \nRachel's units gain +%1% firepower and +%2% defence.");
        text = replaceTextArgs(text, [values[0], CO_RACHEL.d2dCoZoneOffBonus, CO_RACHEL.d2dCoZoneDefBonus]);
        return text;
    };
    this.getPowerDescription = function(co)
    {
        var text = qsTr("Rachel's units gain +%0 luck, +%1% firepower, and +%2% defence.");
        text = replaceTextArgs(text, [CO_RACHEL.powerLuckDamage, CO_RACHEL.powerOffBonus, CO_RACHEL.powerDefBonus]);
        return text;
    };
    this.getPowerName = function(co)
    {
        return qsTr("Lucky Lass");
    };
    this.getSuperPowerDescription = function(co)
    {
        var text = qsTr("Rachel launches three missiles from Orange Star HQ in Omega Land. The missiles deal -%0 HP of damage to all units in a 2-space radius and target the enemy's greatest accumulation of footsoldier HP, then unit HP, then unit value. Rachel's units gain +%1% firepower and +%2% defence.");
        text = replaceTextArgs(text, [CO_RACHEL.superPowerDamage, CO_RACHEL.powerOffBonus, CO_RACHEL.powerDefBonus]);
        return text;

    };
    this.getSuperPowerName = function(co)
    {
        return qsTr("Covering Fire");
    };
    this.getPowerSentences = function(co)
    {
        return [qsTr("I will protect this land to the end!"),
                qsTr("This time, I'm serious!"),
                qsTr("You don't have a chance!"),
                qsTr("I'm going to send you packing!"),
                qsTr("Don't pick a fight with me!"),
                qsTr("Finally, some real competition!")];
    };
    this.getVictorySentences = function(co)
    {
        return [qsTr("Another one down... Who's next?"),
                qsTr("I never give up."),
                qsTr("Don't take me lightly just because I'm cute!")];
    };
    this.getDefeatSentences = function(co)
    {
        return [qsTr("You beat me? Hmmm... Not bad!"),
                qsTr("You've got to be kidding me!")];
    };
    this.getName = function()
    {
        return qsTr("Rachel");
    };
    this.getAiUsePower = function(co, powerSurplus, turnMode)
    {
        // scop spam
        if (co.canUseSuperpower())
        {
            return GameEnums.PowerMode_Superpower;
        }
        return GameEnums.PowerMode_Off;
    };
}

Constructor.prototype = CO;
var CO_RACHEL = new Constructor();
