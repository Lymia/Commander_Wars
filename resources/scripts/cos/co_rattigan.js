var Constructor = function()
{
    this.init = function(co, map)
    {
        co.setPowerStars(3);
        co.setSuperpowerStars(4);
    };

    this.getCOStyles = function()
    {
        return ["+alt"];
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
                animation.setSound("power10_1.wav", 1, delay);
            }
            else
            {
                animation.setSound("power10_2.wav", 1, delay);
            }
            if (animations.length < 5)
            {
                animation.addSprite("power10", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                powerNameAnimation.queueAnimation(animation);
                animations.push(animation);
            }
            else
            {
                animation.addSprite("power10", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
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

        CO_RATTIGAN.rattiganDamage(co, CO_RATTIGAN.superPowerDamage, dialogAnimation, map);
    };

    this.rattiganDamage = function(co, value, animation2, map)
    {
        var player = co.getOwner();
        var counter = 0;
        var playerCounter = map.getPlayerCount();
        var animation = null;
        var animations = [];

        for (var i2 = 0; i2 < playerCounter; i2++)
        {
            var enemyPlayer = map.getPlayer(i2);
            if ((enemyPlayer !== player) &&
                (player.checkAlliance(enemyPlayer) === GameEnums.Alliance_Enemy))
            {

                var units = enemyPlayer.getUnits();
                units.randomize();
                for (i = 0; i < units.size(); i++)
                {
                    var unit = units.at(i);

                    animation = GameAnimationFactory.createAnimation(map, unit.getX(), unit.getY());
                    var delay = globals.randInt(135, 265);
                    if (animations.length < 5)
                    {
                        delay *= i;
                    }
                    animation.setSound("power4.wav", 1, delay);
                    if (animations.length < 5)
                    {
                        animation.addSprite("power4", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                        animation2.queueAnimation(animation);
                        animations.push(animation);
                    }
                    else
                    {
                        animation.addSprite("power4", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                        animations[counter].queueAnimation(animation);
                        animations[counter] = animation;
                        counter++;
                        if (counter >= animations.length)
                        {
                            counter = 0;
                        }
                    }
                    animation.writeDataInt32(unit.getX());
                    animation.writeDataInt32(unit.getY());
                    animation.writeDataInt32(value);
                    animation.setEndOfAnimationCall("ANIMATION", "postAnimationDamage");
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
                audio.addMusic("resources/music/cos/power.ogg", 992, 45321);
                break;
            case GameEnums.PowerMode_Superpower:
                audio.addMusic("resources/music/cos/superpower.ogg", 1505, 49515);
                break;
            case GameEnums.PowerMode_Tagpower:
                audio.addMusic("resources/music/cos/tagpower.ogg", 14611, 65538);
                break;
            default:
                audio.addMusic("resources/music/cos/rattigan.ogg")
                break;
            }
        }
    };

    this.getCOUnitRange = function(co, map)
    {
        return 3;
    };
    this.getCOArmy = function()
    {
        return "YC";
    };

    this.superPowerDamage = 1;
    this.superPowerOffBonus = 50;
    this.superPowerDefBonus = 30;

    this.powerOffBonus = 30;
    this.powerBaseOffBonus = 10;
    this.powerBaseDefBonus = 10;
    this.powerMultiplier = 5;
    this.powerMovementPoints = 1;

    this.d2dOffBonus = 25;
    this.d2dOffMalus = -10;
    this.d2dDefBonus = 0;

    this.d2dCoZoneDefBonus = 10;
    this.d2dCoZoneOffBonus = 50;
    this.d2dCoZoneOffMalus = 0;

    this.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                 defender, defPosX, defPosY, isDefender, action, luckmode, map)
    {
        if (CO.isActive(co))
        {
            var count = CO_RATTIGAN.getUnitCount(co, defPosX, defPosY, map);
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                if (count > 0)
                {
                    return CO_RATTIGAN.superPowerOffBonus + count * CO_RATTIGAN.powerMultiplier;
                }
                return CO_RATTIGAN.powerBaseOffBonus;
            case GameEnums.PowerMode_Power:
                if (count > 0)
                {
                    return CO_RATTIGAN.powerOffBonus + count * CO_RATTIGAN.powerMultiplier;
                }
                return CO_RATTIGAN.powerBaseOffBonus;
            default:
                if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                {
                    if (count > 0)
                    {
                        return CO_RATTIGAN.d2dCoZoneOffBonus;
                    }
                    return CO_RATTIGAN.d2dCoZoneOffMalus;
                }
                else if (map === null ||
                         (map !== null && map.getGameRules().getCoGlobalD2D()))
                {
                    if (count > 0)
                    {
                        return CO_RATTIGAN.d2dOffBonus;
                    }
                    else
                    {
                        return CO_RATTIGAN.d2dOffMalus;
                    }
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
            var count = CO_RATTIGAN.getUnitCount(co, defPosX, defPosY, map);
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                if (count > 0)
                {
                    return CO_RATTIGAN.superPowerDefBonus;
                }
                return CO_RATTIGAN.powerBaseDefBonus;
            case GameEnums.PowerMode_Power:
                return CO_RATTIGAN.powerBaseDefBonus;
            default:
                if (co.inCORange(Qt.point(defPosX, defPosY), defender))
                {
                    return CO_RATTIGAN.d2dCoZoneDefBonus;
                }
                else if (map === null ||
                         (map !== null && map.getGameRules().getCoGlobalD2D()))
                {
                    return CO_RATTIGAN.d2dDefBonus;
                }
            }
        }
        return 0;
    };

    this.getUnitCount = function(co, x, y, map)
    {
        if (x >= 0 && y >= 0)
        {
            var count = 0;
            var fields = globals.getCircle(1, 2);
            var size = fields.size();
            for (var i = 0; i < size; i++)
            {
                var point = fields.at(i);
                if (map.onMap(x + point.x, y + point.y))
                {
                    var unit = map.getTerrain(x + point.x, y + point.y).getUnit();
                    if ((unit !== null) &&
                            (co.getOwner().isEnemyUnit(unit)))
                    {
                        count++;
                    }
                }
            }
        }
        return count;
    };

    this.getMovementpointModifier = function(co, unit, posX, posY, map)
    {
        if (CO.isActive(co))
        {
            if (co.getPowerMode() === GameEnums.PowerMode_Power)
            {
                return CO_RATTIGAN.powerMovementPoints;
            }
        }
        return 0;
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
                return ["ZCOUNIT_AUTO_TANK"];
            }
        }
        return [];
    };

    // CO - Intel
    this.getBio = function(co)
    {
        return qsTr("A dutiful CO of the Yellow Comet Army addled by a childhood phobia. Blames Sensei for Yellow Comet's lack of former glory.");
    };
    this.getHits = function(co)
    {
        return qsTr("Yellow Comet");
    };
    this.getMiss = function(co)
    {
        return qsTr("Infestations");
    };
    this.getCODescription = function(co)
    {
        return qsTr("Rattigan's troops are trained to be extremely capable at dealing with large regiments of units, but are taken off guard by lone units.");
    };
    this.getLongCODescription = function(co, map)
    {
        var values = [0, 0];
        if (map === null ||
            (map !== null && map.getGameRules().getCoGlobalD2D()))
        {
            values = [CO_RATTIGAN.d2dOffBonus, CO_RATTIGAN.d2dOffMalus];
        }
        var text = qsTr("\nSpecial Unit:\nAuto Tank\n") +
               qsTr("\nGlobal Effect: \nRattigan's units gain +%0% firepower when engaging enemies that have another enemy unit within a 2-space range. They have %1% firepower against enemy units that are otherwise alone.") +
               qsTr("\n\nCO Zone Effect: \nRattigan's units gain +%2% firepower when engaging enemies that have another enemy unit within a 2-space range. They have +%3% firepower against enemy units that are otherwise alone. All of his units gain +%4% defence.");
        text = replaceTextArgs(text, [values[0], values[1], CO_RATTIGAN.d2dCoZoneOffBonus, CO_RATTIGAN.d2dCoZoneOffMalus, CO_RATTIGAN.d2dCoZoneDefBonus]);
        return text;
    };
    this.getPowerDescription = function(co)
    {
        var text = qsTr("Rattigan's units gain +%0 movement and +%2% defence. When attacking enemy units that are not alone, they gain +%3% firepower plus an extra +%4% firepower for each enemy unit within a 2-space radius. Otherwise, they gain +%1% firepower.");
		text = replaceTextArgs(text, [CO_RATTIGAN.powerMovementPoints, CO_RATTIGAN.powerBaseOffBonus, CO_RATTIGAN.powerBaseDefBonus, CO_RATTIGAN.powerOffBonus, CO_RATTIGAN.powerMultiplier]);
        return text;
    };
    this.getPowerName = function(co)
    {
        return qsTr("Raid");
    };
    this.getSuperPowerDescription = function(co)
    {
		var text = qsTr("Rattigan calls in a surprise attack that deals -%0 HP of damage to all enemy units. When attacking enemy units that are not alone, Rattigan's units gain +%1% defence, +%2% firepower, and an extra +%3% firepower for each enemy unit within a 2-space radius. Otherwise, they gain +%4% firepower and +%5% defence.");
		text = replaceTextArgs(text, [CO_RATTIGAN.superPowerDamage, CO_RATTIGAN.superPowerDefBonus, CO_RATTIGAN.superPowerOffBonus, CO_RATTIGAN.powerMultiplier, CO_RATTIGAN.powerBaseOffBonus, CO_RATTIGAN.powerBaseDefBonus]);
        return text;
    };
    this.getSuperPowerName = function(co)
    {
        return qsTr("The Sweep");
    };
    this.getPowerSentences = function(co)
    {
        return [qsTr("All those enemy troops... heh, perfect!"),
                qsTr("G-gah! They're everywhere!"),
                qsTr("Hrumph! This'll learn yeh!"),
                qsTr("Heh, could Sensei do THIS?"),
                qsTr("Who the heck taught you to fight?"),
                qsTr("Stay on your toes! I'm talking to you!")];
    };
    this.getVictorySentences = function(co)
    {
        return [qsTr("With all of those troops, you'd think they'd last longer..."),
                qsTr("A-aughk!! Uhm... I mean... Congratulations, men."),
                qsTr("With Yellow Comet on the line, I can't lose.")];
    };
    this.getDefeatSentences = function(co)
    {
        return [qsTr("What? You had more than enough troops..."),
                qsTr("Sensei! This is all your fault!")];
    };
    this.getName = function()
    {
        return qsTr("Rattigan");
    };
}

Constructor.prototype = CO;
var CO_RATTIGAN = new Constructor();
