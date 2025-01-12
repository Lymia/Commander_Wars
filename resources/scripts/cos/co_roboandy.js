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
            var delay = globals.randInt(135, 265);
            if (animations.length < 5)
            {
                delay *= i;
            }
            animation.setSound("power3.wav", 1, delay);
            if (animations.length < 5)
            {
                animation.addSprite("power3", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
                powerNameAnimation.queueAnimation(animation);
                animations.push(animation);
            }
            else
            {
                animation.addSprite("power3", -map.getImageSize() * 1.27, -map.getImageSize() * 1.27, 0, 2, delay);
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
                audio.addMusic("resources/music/cos/roboandy.ogg", 3653, 46346);
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
        return "MA";
    };

    this.superPowerDamage = 3;
    this.superPowerHeal = 3;
    this.powerDamage = 1;
    this.powerHeal = 1;
    this.powerOffBonus = 10;
    this.powerDefBonus = 10;

    this.d2dOffMissfortuneBonus = 2;
    this.d2dOffBonus = 4;

    this.d2dCoZoneMissfortuneBonus = 4;
    this.d2dCoZoneOffBonus = 6;
    this.d2dCoZoneOffBaseBonus = 10;
    this.d2dCoZoneDefBonus = 10;

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
                return CO_ROBOANDY.powerOffBonus;
            default:
                if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                {
                    return co.getPowerFilled() * CO_ROBOANDY.d2dCoZoneOffBonus + CO_ROBOANDY.d2dCoZoneOffBaseBonus;
                }
                break;
            }
            if (map === null ||
                (map !== null && map.getGameRules().getCoGlobalD2D()))
            {
                return co.getPowerFilled() * CO_ROBOANDY.d2dOffBonus;
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
                return CO_ROBOANDY.powerDefBonus;
            }
            else if (co.inCORange(Qt.point(defPosX, defPosY), defender))
            {
                return CO_ROBOANDY.d2dCoZoneDefBonus;
            }
        }
        return 0;
    };
    this.getBonusMisfortune = function(co, unit, posX, posY, map)
    {
        if (CO.isActive(co))
        {
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
            case GameEnums.PowerMode_Power:
                return 0;
            default:
                if (co.inCORange(Qt.point(posX, posY), unit))
                {
                    return co.getPowerFilled() * CO_ROBOANDY.d2dCoZoneMissfortuneBonus;
                }
                else if (map === null ||
                         (map !== null && map.getGameRules().getCoGlobalD2D()))
                {
                    return co.getPowerFilled() * CO_ROBOANDY.d2dOffMissfortuneBonus;
                }
            }
        }
        return 0;
    };

    this.postBattleActions = function(co, attacker, atkDamage, defender, gotAttacked, weapon, action, map)
    {
        if (CO.isActive(co) && defender !== null)
        {
            var healing = 0;
            var damage = 0;
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
                healing = CO_ROBOANDY.superPowerDamage;
                damage = CO_ROBOANDY.superPowerHeal;
                break;
            case GameEnums.PowerMode_Power:
                healing = CO_ROBOANDY.powerHeal;
                damage = CO_ROBOANDY.powerDamage;
                break;
            default:
                break;
            }
            if (healing > 0 || damage > 0)
            {
                if (attacker.getOwner() === co.getOwner() && attacker.getHp() > 0)
                {
                    attacker.setHp(attacker.getHp() + healing);
                }
                else if (!gotAttacked && attacker.getOwner() === co.getOwner() && attacker.getHp() <= 0)
                {
                    var hp = defender.getHp();
                    if (hp > damage)
                    {
                        defender.setHp(hp - damage);
                    }
                    else
                    {
                        defender.setHp(0.00001);
                    }
                }
                else if (gotAttacked && defender.getOwner() === co.getOwner() && defender.getHp() <= 0)
                {
                    var hp = attacker.getHp();
                    if (hp > damage)
                    {
                        attacker.setHp(hp - damage);
                    }
                    else
                    {
                        attacker.setHp(0.1);
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
        return qsTr("Lash was asked by the Black Hole Army to create artificial intelligence able to direct troops. However, being given orders by a computer freaked out the soldiers, so she fixed the problem by putting it in an experimental duplicate of Andy from Orange Star.");
    };
    this.getHits = function(co)
    {
        return qsTr("Puppet shows");
    };
    this.getMiss = function(co)
    {
        return qsTr("Housework");
    };
    this.getCODescription = function(co)
    {
        return qsTr("His units gain firepower for his power meter, but he also gains misfortune the more power charge he has.");
    };
    this.getLongCODescription = function(co, map)
    {
        var values = [0, 0];
        if (map === null ||
            (map !== null && map.getGameRules().getCoGlobalD2D()))
        {
            values = [CO_ROBOANDY.d2dOffBonus, CO_ROBOANDY.d2dOffMissfortuneBonus];
        }
        var text = qsTr("\nSpecial Unit:\nAT Cycle\n") +
               qsTr("\nGlobal Effect: \nRobo-Andy's units gain +%0% firepower and +%1 misfortune per CO Power charge star.") +
               qsTr("\n\nCO Zone Effect: \nRobo-Andy's units gain +%2% firepower and +%3% defence. They gain an additional +%4% firepower and +%5 misfortune per CO Power charge star.");
        text = replaceTextArgs(text, [values[0], values[1], CO_ROBOANDY.d2dCoZoneOffBaseBonus, CO_ROBOANDY.d2dCoZoneDefBonus, CO_ROBOANDY.d2dCoZoneOffBonus, CO_ROBOANDY.d2dCoZoneMissfortuneBonus]);
        return text;
    };
    this.getPowerDescription = function(co)
    {
        var text = qsTr("Robo-Andy's units gain +%0% firepower and +%1% defence. His units heal +%2 HP after any attack or counterattack, and deal -%3 HP of damage to their attacker if they are destroyed.");
        text = replaceTextArgs(text, [CO_ROBOANDY.powerOffBonus, CO_ROBOANDY.powerDefBonus, CO_ROBOANDY.powerHeal, CO_ROBOANDY.powerDamage]);
        return text;
    };
    this.getPowerName = function(co)
    {
        return qsTr("Cooldown");
    };
    this.getSuperPowerDescription = function(co)
    {
        var text = qsTr("Robo-Andy's units gain +%0% firepower and +%1% defence. His units heal +%2 HP after any attack or counterattack, and deal -%3 HP of damage to their attacker if they are destroyed.");
        text = replaceTextArgs(text, [CO_ROBOANDY.powerOffBonus, CO_ROBOANDY.powerDefBonus, CO_ROBOANDY.superPowerHeal, CO_ROBOANDY.superPowerDamage]);
        return text;
    };
    this.getSuperPowerName = function(co)
    {
        return qsTr("Critical Mass");
    };
    this.getPowerSentences = function(co)
    {
        return [qsTr("DADADA! ANNOYING PESTS DESERVE TO BE SWATTED!"),
                qsTr("ERROR DETECTED. PREPARE TO BE ELIMINATED!"),
                qsTr("YOU'RE A RUDE ONE! EAT HEAVY METAL!"),
                qsTr("KYAAA! SUCH A JERK! TIME TO DIE!"),
                qsTr("WARNING! WARNING! EXECUTING EMERGENCY VENTING MODULE."),
                qsTr("YOU'RE SO DISAPPOINTING. WITNESS MY POWER!")];
    };
    this.getVictorySentences = function(co)
    {
        return [qsTr("USELESS CREATURE!"),
                qsTr("THANK YOU FOR USING ROBO-ANDY, INFERIOR BEING!"),
                qsTr("FOR MORE EXCITING PERFORMANCES PLEASE UPGRADE TO THE LATEST VERSION.")];
    };
    this.getDefeatSentences = function(co)
    {
        return [qsTr("HOW MAY I SERVE YOU, INFERIOR BEING?"),
                qsTr("ALT+TAB! ALT+TAB! ALT+TAB!")];
    };
    this.getName = function()
    {
        return qsTr("Robo-Andy");
    };
}

Constructor.prototype = CO;
var CO_ROBOANDY = new Constructor();
