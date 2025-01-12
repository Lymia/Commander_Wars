var Constructor = function()
{
    this.getCOStyles = function()
    {
        return ["+alt", "+alt2"];
    };

    this.init = function(co, map)
    {
        co.setPowerStars(5);
        co.setSuperpowerStars(5);
    };

    this.loadCOMusic = function(co, map)
    {
        if (CO.isActive(co))
        {
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Power:
                audio.addMusic("resources/music/cos/bh_power.ogg", 1091 , 49930);
                break;
            case GameEnums.PowerMode_Superpower:
                audio.addMusic("resources/music/cos/bh_superpower.ogg", 3161 , 37731);
                break;
            case GameEnums.PowerMode_Tagpower:
                audio.addMusic("resources/music/cos/bh_tagpower.ogg", 779 , 51141);
                break;
            default:
                audio.addMusic("resources/music/cos/sturm.ogg", 4054, 60338);
                break;
            }
        }
    };

    this.activatePower = function(co, map)
    {
        var dialogAnimation = co.createPowerSentence();
        var powerNameAnimation = co.createPowerScreen(GameEnums.PowerMode_Power);
        dialogAnimation.queueAnimation(powerNameAnimation);

        CO_STURM.throwMeteor(co, CO_STURM.powerDamage, powerNameAnimation, map);
    };

    this.activateSuperpower = function(co, powerMode, map)
    {
        var dialogAnimation = co.createPowerSentence();
        var powerNameAnimation = co.createPowerScreen(powerMode);
        powerNameAnimation.queueAnimationBefore(dialogAnimation);

        CO_STURM.throwMeteor(co, CO_STURM.superPowerDamage, powerNameAnimation, map);
    };

    this.throwMeteor = function(co, damage, powerNameAnimation, map)
    {
        var meteorTarget = co.getOwner().getRockettarget(2, damage);
        // create cool meteor animation :)
        var animation = GameAnimationFactory.createAnimation(map, meteorTarget.x + 2, meteorTarget.y - 4);
        animation.addSprite("meteor", 0, 0, 2500, 4.0);
        animation.addTweenPosition(Qt.point((meteorTarget.x - 2) * map.getImageSize(), (meteorTarget.y - 2) * map.getImageSize()), 1000);
        animation.addTweenScale(0.65, 1000);
        animation.addTweenColor(0, "#FFFFFFFF", "#00FFFFFF", 1000, false, 1200);
        animation.addSound("meteorFall.wav");
        powerNameAnimation.queueAnimation(animation);
        var animation2 = GameAnimationFactory.createAnimation(map, 0, 0);
        animation2.addSprite2("white_pixel", 0, 0, 4200, map.getMapWidth(), map.getMapHeight());
        animation2.addTweenColor(0, "#00FFFFFF", "#FFFFFFFF", 3000, true, 1000);
        animation2.addSound("meteorImpact.wav");
        powerNameAnimation.queueAnimation(animation2);
        animation.setEndOfAnimationCall("CO_STURM", "postAnimationThrowMeteor");
        animation.setStartOfAnimationCall("CO_STURM", "preAnimationThrowMeteor");
        CO_STURM.postAnimationThrowMeteorTarget = meteorTarget;
        CO_STURM.postAnimationThrowMeteorDamage = damage;
    };
    this.preAnimationThrowMeteor = function(animation, map)
    {
        map.centerMap(CO_STURM.postAnimationThrowMeteorTarget.x, CO_STURM.postAnimationThrowMeteorTarget.y);
    };
    this.postAnimationThrowMeteorTarget = null;
    this.postAnimationThrowMeteorDamage = 0;
    this.postAnimationThrowMeteor = function(animation, map)
    {
        var meteorTarget = CO_STURM.postAnimationThrowMeteorTarget;
        var damage = CO_STURM.postAnimationThrowMeteorDamage;
        var fields = globals.getCircle(0, 2);
        // check all target fields
        var size = fields.size();
        for (var i = 0; i < size; i++)
        {
            var x = fields.at(i).x + meteorTarget.x;
            var y = fields.at(i).y + meteorTarget.y;
            // check if the target is on the map
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
        CO_STURM.postAnimationThrowMeteorTarget = null;
        CO_STURM.postAnimationThrowMeteorDamage = 0;
    };

    this.getCOUnitRange = function(co, map)
    {
        return 3;
    };
    this.getCOArmy = function()
    {
        return "BH";
    };

    this.superPowerDamage = 8;
    this.superPowerOffBonus = 60;
    this.superPowerDefBonus = 60;

    this.powerDamage = 4;
    this.powerOffBonus = 40;
    this.powerDefBonus = 40;

    this.d2dCoZoneOffBonus = 40;
    this.d2dCoZoneDefBonus = 40;

    this.d2dOffBonus = 20;
    this.d2dDefBonus = 20;

    this.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                 defender, defPosX, defPosY, isDefender, action, luckmode, map)
    {
        if (CO.isActive(co))
        {
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                return CO_STURM.superPowerOffBonus;
            case GameEnums.PowerMode_Power:
                return CO_STURM.powerOffBonus;
            default:
                if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                {
                    return CO_STURM.d2dCoZoneOffBonus;
                }
                else if (map === null ||
                         (map !== null && map.getGameRules().getCoGlobalD2D()))
                {
                    return CO_STURM.d2dOffBonus;
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
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                return CO_STURM.superPowerDefBonus;
            case GameEnums.PowerMode_Power:
                return CO_STURM.powerDefBonus;
            default:
                if (co.inCORange(Qt.point(defPosX, defPosY), defender))
                {
                    return CO_STURM.d2dCoZoneDefBonus;
                }
                else if (map === null ||
                         (map !== null && map.getGameRules().getCoGlobalD2D()))
                {
                    return CO_STURM.d2dDefBonus;
                }
            }
        }
        return 0;
    };
    this.getMovementcostModifier = function(co, unit, posX, posY, map)
    {
        if (CO.isActive(co))
        {
            if (unit.getOwner() === co.getOwner())
            {
                if (map === null ||
                    (map !== null && map.getGameRules().getCoGlobalD2D()))
                {
                    if (map.getGameRules().getCurrentWeather().getWeatherId() === "WEATHER_SNOW")
                    {
                        return 0;
                    }
                    else
                    {
                        return -999;
                    }
                }
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
        return qsTr("The original commander of the Black Hole Army. A mysterious invader from another world. Mastermind of the Cosmo and Macro wars.");
    };
    this.getHits = function(co)
    {
        return qsTr("Plotting invasions");
    };
    this.getMiss = function(co)
    {
        return qsTr("Peace");
    };
    this.getCODescription = function(co)
    {
        return qsTr("His superior troops are not affected by terrain. Only snow can stop him.");
    };
    this.getLongCODescription = function(co, map)
    {
        var values = [0, 0];
        if (map === null ||
            (map !== null && map.getGameRules().getCoGlobalD2D()))
        {
            values = [CO_STURM.d2dOffBonus, CO_STURM.d2dDefBonus];
        }
        var text = qsTr("\nGlobal Effect: \nSturm's units are unhindered by terrain unless the weather is Snow. They gain +%0% firepower and +%1% defence.") +
                   qsTr("\n\nCO Zone Effect: \nSturm's units gain +%2% firepower and +%3% defence.");
        text = replaceTextArgs(text, [values[0], values[1], CO_STURM.d2dCoZoneOffBonus, CO_STURM.d2dCoZoneDefBonus]);
        return text;
    };
    this.getPowerDescription = function(co)
    {
        var text = qsTr("A meteor falls from space onto the largest accumulation of enemy funds in a 2-space radius, dealing -%0 HP of damage to all affected units. Sturm's units gain +%1% firepower and +%2% defence.");
        text = replaceTextArgs(text, [CO_STURM.powerDamage, CO_STURM.powerOffBonus, CO_STURM.powerDefBonus]);
        return text;
    };
    this.getPowerName = function(co)
    {
        return qsTr("Meteor Slam");
    };
    this.getSuperPowerDescription = function(co)
    {
        var text = qsTr("A massive meteor falls from space onto the largest accumulation of enemy funds in a 2-space radius, dealing -%0 HP of damage to all affected units. Sturm's units gain +%1% firepower and +%2% defence.");
        text = replaceTextArgs(text, [CO_STURM.superPowerDamage, CO_STURM.superPowerOffBonus, CO_STURM.superPowerDefBonus]);
        return text;
    };
    this.getSuperPowerName = function(co)
    {
        return qsTr("Meteor Strike");
    };
    this.getPowerSentences = function(co)
    {
        return [qsTr("Prepare to embrace darkness!"),
                qsTr("You will tremble before my power!"),
                qsTr("Fear is all you have left..."),
                qsTr("You shall not survive!"),
                qsTr("Burning earth!"),
                qsTr("Such power... I regret crushing it.")];
    };
    this.getVictorySentences = function(co)
    {
        return [qsTr("What foolish worm thinks they can oppose me?"),
                qsTr("My name is Sturm. Hear it and tremble."),
                qsTr("This is but a taste of my power!")];
    };
    this.getDefeatSentences = function(co)
    {
        return [qsTr("Gwaaaaaaaaaahhhh! I underestimated the strength of these worms!"),
                qsTr("NOOOOOOOOOOOOOOOOOOO!!!")];
    };
    this.getName = function()
    {
        return qsTr("Sturm");
    };
}

Constructor.prototype = CO;
var CO_STURM = new Constructor();
