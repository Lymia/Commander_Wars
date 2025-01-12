var Constructor = function()
{
    this.init = function(co, map)
    {
        co.setPowerStars(3);
        co.setSuperpowerStars(3);
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
            animation.writeDataInt32(unit.getX());
            animation.writeDataInt32(unit.getY());
            animation.writeDataInt32(CO_BEAST.powerHeal);
            animation.setEndOfAnimationCall("ANIMATION", "postAnimationHeal");
            var delay = globals.randInt(135, 265);
            if (animations.length < 5)
            {
                delay *= i;
            }
            animation.setSound("power0.wav", 1, delay);
            if (animations.length < 5)
            {
                animation.addSprite("power0", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                powerNameAnimation.queueAnimation(animation);
                animations.push(animation);
            }
            else
            {
                animation.addSprite("power0", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
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

        var units = co.getOwner().getUnits();
        var animations = [];
        var counter = 0;
        units.randomize();
        for (var i = 0; i < units.size(); i++)
        {
            var unit = units.at(i);

            var animation = GameAnimationFactory.createAnimation(map, unit.getX(), unit.getY());
            animation.writeDataInt32(unit.getX());
            animation.writeDataInt32(unit.getY());
            animation.writeDataInt32(CO_BEAST.superPowerHeal);
            animation.setEndOfAnimationCall("ANIMATION", "postAnimationHeal");
            var delay = globals.randInt(135, 265);
            if (animations.length < 7)
            {
                delay *= i;
            }
            if (i % 2 === 0)
            {
                animation.setSound("power12_1.wav", 1, delay);
            }
            else
            {
                animation.setSound("power12_2.wav", 1, delay);
            }
            if (animations.length < 7)
            {
                animation.addSprite("power12", -map.getImageSize() * 2, -map.getImageSize() * 2, 0, 2, delay);
                powerNameAnimation.queueAnimation(animation);
                animations.push(animation);
            }
            else
            {
                animation.addSprite("power12", -map.getImageSize() * 2, -map.getImageSize() * 2, 0, 2, delay);
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

    this.loadCOMusic = function(co, map)
    {
        if (CO.isActive(co))
        {
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Power:
                audio.addMusic("resources/music/cos/power_ids_dc.ogg", 0, 0);
                break;
            case GameEnums.PowerMode_Superpower:
                audio.addMusic("resources/music/cos/power_ids_dc.ogg", 0, 0);
                break;
            case GameEnums.PowerMode_Tagpower:
                audio.addMusic("resources/music/cos/bh_tagpower.ogg", 779 , 51141);
                break;
            default:
                audio.addMusic("resources/music/cos/beast.ogg",  0, 0);
                break;
            }
        }
    };

    this.getCOUnitRange = function(co, map)
    {
        return 2;
    };
    this.getCOArmy = function()
    {
        return "DM";
    };

    this.superPowerHeal = 3;
    this.superPowerMovementBonus = 2;

    this.powerHeal = 2;
    this.powerOffBonus = 60;
    this.powerDefBonus = 10;
    this.powerSelfDamage = 1;

    this.d2dCoZoneBaseOffBonus = 10;
    this.d2dCoZoneOffBonus = 60;
    this.d2dCoZoneDefBonus = 10;
    this.d2dCoZoneSelfDamage = 1;

    this.d2dOffBonus = 30;
    this.d2dSelfDamage = 1;

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
                if (!isDefender)
                {
                    return CO_BEAST.powerOffBonus;
                }
                break;
            default:
                if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                {
                    if (!isDefender)
                    {
                        return CO_BEAST.d2dCoZoneOffBonus;
                    }
                    return CO_BEAST.d2dCoZoneBaseOffBonus;
                }
                if (map === null ||
                    (map !== null && map.getGameRules().getCoGlobalD2D()))
                {
                    return CO_BEAST.d2dOffBonus;
                }
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
                return CO_BEAST.powerDefBonus;
            }
            else if (co.inCORange(Qt.point(defPosX, defPosY), defender))
            {
                return CO_BEAST.d2dCoZoneDefBonus;
            }
        }
        return 0;
    };
    this.getMovementpointModifier = function(co, unit, posX, posY, map)
    {
        if (CO.isActive(co))
        {
            if (co.getPowerMode() === GameEnums.PowerMode_Superpower ||
                    co.getPowerMode() === GameEnums.PowerMode_Tagpower)
            {
                return CO_BEAST.superPowerMovementBonus;
            }
        }
        return 0;
    };
    this.postBattleActions = function(co, attacker, atkDamage, defender, gotAttacked, weapon, action, map)
    {
        if (CO.isActive(co))
        {
            var selfDamage = 0;
            if (map === null ||
                (map !== null && map.getGameRules().getCoGlobalD2D()))
            {
                selfDamage = CO_BEAST.d2dSelfDamage;
            }
            if (co.getPowerMode() > GameEnums.PowerMode_Off)
            {
                selfDamage = CO_BEAST.powerSelfDamage;
            }
            else if (co.inCORange(Qt.point(attacker.getX(), attacker.getY()), attacker))
            {
                selfDamage = CO_BEAST.d2dCoZoneSelfDamage;
            }
            if (selfDamage > 0)
            {
                if (attacker.getOwner() === co.getOwner() && attacker.getHp() > 0)
                {
                    var hp = attacker.getHp();
                    if (hp > selfDamage)
                    {
                        attacker.setHp(hp - selfDamage);
                    }
                    else
                    {
                        attacker.setHp(0.00001);
                    }
                }
            }
        }
    };
    this.getAiCoUnitBonus = function(co, unit, map)
    {
        return 1;
    };
    this.getCOUnits = function(co, building, map)
    {
        if (CO.isActive(co))
        {
            var buildingId = building.getBuildingID();
            if (buildingId === "FACTORY" ||
                buildingId === "TOWN" ||
                BUILDING.isHq(building))
            {
                return ["ZCOUNIT_AT_CYCLE"];
            }
        }
        return [];
    };

    // CO - Intel
    this.getBio = function(co)
    {
        return qsTr("A bandit working for Caulder. Believes that there is no law but 'The Beast's Law'. Driven by rage and greed.");
    };
    this.getHits = function(co)
    {
        return qsTr("Looting, robbing, and killing");
    };
    this.getMiss = function(co)
    {
        return qsTr("Duty and honor");
    };
    this.getCODescription = function(co)
    {
        return qsTr("His units have high firepower, but their reckless tendencies often get them hurt.");
    };
    this.getLongCODescription = function(co, map)
    {
        var values = [0, 0];
        if (map === null ||
            (map !== null && map.getGameRules().getCoGlobalD2D()))
        {
            values = [CO_BEAST.d2dOffBonus, CO_BEAST.d2dSelfDamage];
        }
        var text = qsTr("\nSpecial Unit:\nAT Cycle\n") +
                   qsTr("\nGlobal Effect: \nThe Beast's units gain +%0% firepower when attacking but also receive -%1 HP of extra damage in recoil.") +
                   qsTr("\n\nCO Zone Effect: \nThe Beast's units gain +%2% firepower when attacking and +%4% defence, but also receive -%3 HP of extra damage after attacking. His units gain +%5% firepower when not attacking.");
        text = replaceTextArgs(text, [values[0], values[1], CO_BEAST.d2dCoZoneOffBonus, CO_BEAST.d2dCoZoneSelfDamage, CO_BEAST.d2dCoZoneDefBonus, CO_BEAST.d2dCoZoneBaseOffBonus]);
        return text;
    };
    this.getPowerDescription = function(co)
    {
        var text = qsTr("Restores %0 HP to all of The Beast's units. His units get +%1% firepower while attacking and +%2% defense, but also receive -%3 HP of extra damage after attacking.");
        text = replaceTextArgs(text, [CO_BEAST.powerHeal, CO_BEAST.powerOffBonus, CO_BEAST.powerDefBonus, CO_BEAST.powerSelfDamage]);
        return text;
    };
    this.getPowerName = function(co)
    {
        return qsTr("Steroids");
    };
    this.getSuperPowerDescription = function(co)
    {
        var text = qsTr("Restores %0 HP to all of The Beast's units. His units get +%1 movement, +%2% firepower while attacking, and +%3% defense, but also receive -%4 HP of extra damage after attacking.");
        text = replaceTextArgs(text, [CO_BEAST.superPowerHeal, CO_BEAST.superPowerMovementBonus, CO_BEAST.powerOffBonus, CO_BEAST.powerDefBonus, CO_BEAST.powerSelfDamage]);
        return text;
    };
    this.getSuperPowerName = function(co)
    {
        return qsTr("The Beast's Law");
    };
    this.getPowerSentences = function(co)
    {
        return [qsTr("The Beast will tear you to shreds!"),
                qsTr("I need a place to aim!"),
                qsTr("Leave their corpses where they fall!"),
                qsTr("Graaahhhh!!"),
                qsTr("Kill anyone who stands in the way!")];
    };
    this.getVictorySentences = function(co)
    {
        return [qsTr("Gwar har har har!"),
                qsTr("You can't stand up to The Beast!"),
                qsTr("There's no law like The Beast's law!")];
    };
    this.getDefeatSentences = function(co)
    {
        return [qsTr("You ain't seen the last of The Beast!"),
                qsTr("Head for the hills, roaches! We're outmanned!")];
    };
    this.getName = function()
    {
        return qsTr("Beast");
    };
}

Constructor.prototype = CO;
var CO_BEAST = new Constructor();
