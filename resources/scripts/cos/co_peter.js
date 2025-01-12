var Constructor = function()
{
    this.init = function(co, map)
    {
        co.setPowerStars(2);
        co.setSuperpowerStars(3);
    };

    this.getCOStyles = function()
    {
        return ["+alt"];
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
                audio.addMusic("resources/music/cos/peter.ogg", 45424, 90079);
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
                animation.setSound("power8_1.wav", 1, delay);
            }
            else
            {
                animation.setSound("power8_2.wav", 1, delay);
            }
            if (animations.length < 5)
            {
                animation.addSprite("power8", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                powerNameAnimation.queueAnimation(animation);
                animations.push(animation);
            }
            else
            {
                animation.addSprite("power8", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
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

    this.peterDamage = function(co, value, animation2, map)
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
                    if (map.getTerrain(unit.getX(), unit.getY()).getBuilding() === null)
                    {
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
        }
    };

    this.activateSuperpower = function(co, powerMode, map)
    {
        var dialogAnimation = co.createPowerSentence();
        var powerNameAnimation = co.createPowerScreen(powerMode);
        powerNameAnimation.queueAnimationBefore(dialogAnimation);

        CO_PETER.peterDamage(co, CO_PETER.superPowerDamage, powerNameAnimation, map);
    };

    this.getCOUnitRange = function(co, map)
    {
        return 3;
    };
    this.getCOArmy = function()
    {
        return "GE";
    };

    this.superPowerDamage = 2;
    this.superPowerOffBonus = 60

    this.powerBaseOffBonus = 10;
    this.powerOffBonus = 50;
    this.powerDefBonus = 10;

    this.d2dCoZoneOffBonus = 50;
    this.d2dCoZoneBaseOffBonus = 10;
    this.d2dCoZoneDefBonus = 10;

    this.d2dOffBonus = 20;
    this.d2dTerrainMalus = 5;

    this.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                      defender, defPosX, defPosY, isDefender, action, luckmode, map)
    {
        if (CO.isActive(co))
        {
            var directEnemy = false;
            if (defender !== null)
            {
                directEnemy = (defender.getBaseMaxRange() === 1);
            }
            var bonus = 0;
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                if (directEnemy)
                {
                    bonus += CO_PETER.superPowerOffBonus;
                }
                bonus += CO_PETER.powerBaseOffBonus;
                break;
            case GameEnums.PowerMode_Power:
                if (directEnemy)
                {
                    bonus += CO_PETER.powerOffBonus;
                }
                bonus += CO_PETER.powerBaseOffBonus;
                break;
            default:
                if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                {
                    if (directEnemy)
                    {
                        bonus = CO_PETER.d2dCoZoneOffBonus;
                    }
                    bonus += CO_PETER.d2dCoZoneBaseOffBonus;
                }
                else if (map === null ||
                         (map !== null && map.getGameRules().getCoGlobalD2D()))
                {
                    if (directEnemy)
                    {
                        bonus = CO_PETER.d2dOffBonus;
                    }
                }
                break;
            }
            if (defender !== null)
            {
                bonus -= defender.getTerrainDefense() * CO_PETER.d2dTerrainMalus;
            }
            return bonus;
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
                return CO_PETER.powerDefBonus;
            }
            else if (co.inCORange(Qt.point(defPosX, defPosY), defender))
            {
                return CO_PETER.d2dCoZoneDefBonus;
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
                return ["ZCOUNIT_ROYAL_GUARD"];
            }
        }
        return [];
    };
    // CO - Intel
    this.getBio = function(co)
    {
        return qsTr("A veteran CO with over 30 years of experience, mostly from the plains of Cosmo Land. Cautious but decisive.");
    };
    this.getHits = function(co)
    {
        return qsTr("Electric razors");
    };
    this.getMiss = function(co)
    {
        return qsTr("Excuses");
    };
    this.getCODescription = function(co)
    {
        return qsTr("Peter's units hit enemy direct-combat units harder. However, enemies in thicker terrain resist his attacks.");
    };
    this.getLongCODescription = function(co, map)
    {
        var values = [0];
        if (map === null ||
            (map !== null && map.getGameRules().getCoGlobalD2D()))
        {
            values = [CO_PETER.d2dOffBonus, CO_PETER.d2dTerrainMalus];
        }
        var text = qsTr("\nSpecial Unit:\nRoyal Guard\n") +
                   qsTr("\nGlobal Effect: \nPeter's units gain +%0% firepower against enemy direct units. Peter's units lose -%1% firepower for each terrain star the enemy has.") +
                   qsTr("\n\nCO Zone Effect: \nPeter's units gain +%3% firepower and +%4% defence. They gain an additional +%2% firepower against enemy direct units.");
        text = replaceTextArgs(text, [values[0], values[1], CO_PETER.d2dCoZoneOffBonus, CO_PETER.d2dCoZoneBaseOffBonus, CO_PETER.d2dCoZoneDefBonus]);
        return text;
    };
    this.getPowerDescription = function(co)
    {
		var text = qsTr("Peter's units gain +%0% firepower and +%1% defence. They gain an additional +%2% firepower against enemy direct units.");
		text = replaceTextArgs(text, [CO_PETER.powerBaseOffBonus, CO_PETER.powerDefBonus, CO_PETER.powerOffBonus]);
        return text;
    };
    this.getPowerName = function(co)
    {
        return qsTr("General Offensive");
    };
    this.getSuperPowerDescription = function(co)
    {
        var text = qsTr("Peter commands an assault that deals -%0 HP of damage to all enemy units not on a property. His units gain +%1% firepower and +%2% defence. They gain an additional +%3% firepower against enemy direct units.");
		text = replaceTextArgs(text, [CO_PETER.superPowerDamage, CO_PETER.powerBaseOffBonus, CO_PETER.powerDefBonus, CO_PETER.superPowerOffBonus]);
        return text;
    };
    this.getSuperPowerName = function(co)
    {
        return qsTr("Bombing Run");
    };
    this.getPowerSentences = function(co)
    {
        return [qsTr("We're going in, boys. Wait for the signal, then strike!"),
                qsTr("You won't even have time to take cover."),
                qsTr("It's time to tear through their tanks."),
                qsTr("Bogged down? Me? Ha! You seriously overestimate yourself."),
                qsTr("I've been down there with the grunts... I know what's feasible and what's not."),
                qsTr("I may not be unbeatable, but what you've got is pathetic.")];
    };
    this.getVictorySentences = function(co)
    {
        return [qsTr("Experience is the key to my success."),
                qsTr("I hope you learned something from this."),
                qsTr("What did you think you were doing with those tanks?")];
    };
    this.getDefeatSentences = function(co)
    {
        return [qsTr("Your tanks were better than mine."),
                qsTr("You're never too old to learn some new moves.")];
    };
    this.getName = function()
    {
        return qsTr("Peter");
    };
}

Constructor.prototype = CO;
var CO_PETER = new Constructor();
