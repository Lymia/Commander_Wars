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
                audio.addMusic("resources/music/cos/javier.ogg", 22016, 74716);
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
                animation.setSound("power2_1.wav", 1, delay);
            }
            else
            {
                animation.setSound("power2_2.wav", 1, delay);
            }
            if (animations.length < 5)
            {
                animation.addSprite("power2", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                powerNameAnimation.queueAnimation(animation);
                animations.push(animation);
            }
            else
            {
                animation.addSprite("power2", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
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

    this.getCOUnitRange = function(co, map)
    {
        return 3;
    };
    this.getCOArmy = function()
    {
        return "GE";
    };

    this.superPowerTowerOffBonus = 20;
    this.superPowerTowerDefBonus = 30;
    this.superPowerIndirectDefBonus = 60;

    this.powerTowerOffBonus = 10;
    this.powerTowerDefBonus = 20;
    this.powerIndirectDefBonus = 40;
    this.powerOffBonus = 10;
    this.powerDefBonus = 10;

    this.d2dCoZoneOffBonus = 10;
    this.d2dCoZoneDefBonus = 10;
    this.d2dCoZoneTowerOffBonus = 0;
    this.d2dCoZoneTowerDefBonus = 10;
    this.d2dCoZoneIndirectDefBonus = 40;

    this.d2dTowerOffBonus = 0;
    this.d2dTowerDefBonus = 5;
    this.d2dIndirectDefBonus = 20;


    this.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                      defender, defPosX, defPosY, isDefender, action, luckmode, map)
    {        
        if (CO.isActive(co))
        {
            var towers = co.getOwner().getBuildingCount("TOWER");
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                return towers * CO_JAVIER.superPowerTowerOffBonus + CO_JAVIER.powerOffBonus;
            case GameEnums.PowerMode_Power:
                return towers * CO_JAVIER.powerTowerOffBonus + CO_JAVIER.powerOffBonus;
            default:
                if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                {
                    return CO_JAVIER.d2dCoZoneOffBonus + towers * CO_JAVIER.d2dCoZoneTowerOffBonus;
                }
                else if (map === null ||
                         (map !== null && map.getGameRules().getCoGlobalD2D()))
                {
                    return towers * CO_JAVIER.d2dTowerOffBonus;
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
            var rangedAttacked = (Math.abs(atkPosX - defPosX) + Math.abs(atkPosY - defPosY) > 1);
            var towers = co.getOwner().getBuildingCount("TOWER");
            var ret = 0;
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                if (rangedAttacked)
                {
                    ret += CO_JAVIER.superPowerIndirectDefBonus;
                }
                ret += towers * CO_JAVIER.superPowerTowerDefBonus + CO_JAVIER.powerDefBonus;
                break;
            case GameEnums.PowerMode_Power:
                if (rangedAttacked)
                {
                    ret += CO_JAVIER.powerIndirectDefBonus;
                }
                ret += towers * CO_JAVIER.powerTowerDefBonus + CO_JAVIER.powerDefBonus;
                break;
            default:
                if (co.inCORange(Qt.point(defPosX, defPosY), defender))
                {
                    ret += towers * CO_JAVIER.d2dCoZoneTowerDefBonus + CO_JAVIER.d2dCoZoneDefBonus;
                    if (rangedAttacked)
                    {
                        ret += CO_JAVIER.d2dCoZoneIndirectDefBonus;
                    }
                }
                else if (map === null ||
                        (map !== null && map.getGameRules().getCoGlobalD2D()))
                {
                    ret += towers * CO_JAVIER.d2dTowerDefBonus;
                    if (rangedAttacked)
                    {
                        ret += CO_JAVIER.d2dIndirectDefBonus;
                    }
                }
                break;
            }
        }
        return ret;
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
                return ["ZCOUNIT_CHAPERON"];
            }
        }
        return [];
    };

    // CO - Intel
    this.getBio = function(co)
    {
        return qsTr("A Green Earth commander who values chivalry and honor above all else. Often orders his units to charge.");
    };
    this.getHits = function(co)
    {
        return qsTr("Honor");
    };
    this.getMiss = function(co)
    {
        return qsTr("Retreating");
    };
    this.getCODescription = function(co)
    {
        return qsTr("His units possess superior defences against indirect attacks. Communications Towers also grant his units defence.");
    };
    this.getLongCODescription = function(co, map)
    {
        var values = [0, 0, 0];
        if (map === null ||
            (map !== null && map.getGameRules().getCoGlobalD2D()))
        {
            values = [CO_JAVIER.d2dIndirectDefBonus, CO_JAVIER.d2dTowerOffBonus, CO_JAVIER.d2dTowerDefBonus];
        }
        var text = qsTr("\nSpecial Unit:\nChaperon\n") +
                qsTr("\nGlobal Effect: \nJavier's units gain +%0% defence against indirect units. His units gain an additional +%1% firepower and +%2% defence per Communications Tower he owns.") +
                qsTr("\n\nCO Zone Effect: \nJavier's units gain +%6% firepower and +%7% defence. His units gain +%3% defence against indirect units. They gain an additional +%4% firepower and +%5% defence per Communications Tower he owns.");
        text = replaceTextArgs(text, [values[0], values[1], values[2],
                                      CO_JAVIER.d2dCoZoneIndirectDefBonus, CO_JAVIER.d2dCoZoneTowerOffBonus, CO_JAVIER.d2dCoZoneTowerDefBonus, CO_JAVIER.d2dCoZoneOffBonus, CO_JAVIER.d2dCoZoneDefBonus]);
        return text;
    };
    this.getPowerDescription = function(co)
    {
        var text = qsTr("Javier's units gain +%3% firepower and +%4% defence. His units gain +%0% defence against indirect units. They gain an additional +%1% firepower and +%2% defence per Communications Tower he owns.");
        text = replaceTextArgs(text, [CO_JAVIER.powerIndirectDefBonus, CO_JAVIER.powerTowerOffBonus, CO_JAVIER.powerTowerDefBonus, CO_JAVIER.powerOffBonus, CO_JAVIER.powerDefBonus]);
        return text;
    };
    this.getPowerName = function(co)
    {
        return qsTr("Tower Shield");
    };
    this.getSuperPowerDescription = function(co)
    {
        var text = qsTr("Javier's units gain +%3% firepower and +%4% defence. His units gain +%0% defence against indirect units. They gain an additional +%1% firepower and +%2% defence per Communications Tower he owns.");
        text = replaceTextArgs(text, [CO_JAVIER.superPowerIndirectDefBonus, CO_JAVIER.superPowerTowerOffBonus, CO_JAVIER.superPowerTowerDefBonus, CO_JAVIER.powerOffBonus, CO_JAVIER.powerDefBonus]);
        return text;
    };
    this.getSuperPowerName = function(co)
    {
        return qsTr("Tower of Power");
    };
    this.getPowerSentences = function(co)
    {
        return [qsTr("Put away your weapons! You shall not be needing them where you are going!"),
                qsTr("Charge into the toothy maw! For now is the time for a glorious hindspanking!"),
                qsTr("Mighty spire of communication, imbue my blade with stabby might!"),
                qsTr("Your maidenfolk shall soon howl great lamentations!"),
                qsTr("This day shall be the greatest of days, unless tomorrow is even greater!"),
                qsTr("Foe, you are mad if you seek to pierce my steely hide!")];
    };
    this.getVictorySentences = function(co)
    {
        return [qsTr("No sharpness can penetrate my steely hide!"),
                qsTr("Consider your hindquarters righteously spanked!"),
                qsTr("You still live, for Javier is masterful but merciful!")];
    };
    this.getDefeatSentences = function(co)
    {
        return [qsTr("Well done! Your reputation is well deserved."),
                qsTr("I am honored to have served with the noblest knights history has ever seen.")];
    };
    this.getName = function()
    {
        return qsTr("Javier");
    };

}

Constructor.prototype = CO;
var CO_JAVIER = new Constructor();
