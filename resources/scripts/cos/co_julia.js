var Constructor = function()
{
    this.init = function(co, map)
    {
        co.setPowerStars(5);
        co.setSuperpowerStars(5);
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

        CO_JULIA.juliaStun(co, CO_JULIA.powerStunChance, powerNameAnimation, map);
    };

    this.activateSuperpower = function(co, powerMode, map)
    {
        var dialogAnimation = co.createPowerSentence();
        var powerNameAnimation = co.createPowerScreen(powerMode);
        powerNameAnimation.queueAnimationBefore(dialogAnimation);

        CO_JULIA.juliaStun(co, CO_JULIA.superPowerStunChance, powerNameAnimation, map);
    };

    this.juliaStun = function(co, amount, animation2, map)
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
                var count = amount * units.size()
                for (i = 0; i < count; i++)
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
                    animation.setEndOfAnimationCall("CO_JULIA", "postAnimationStun");
                }
            }
        }
    };

    this.postAnimationStun = function(postAnimation, map)
    {
        postAnimation.seekBuffer();
        var x = postAnimation.readDataInt32();
        var y = postAnimation.readDataInt32();
        if (map.onMap(x, y))
        {
            var unit = map.getTerrain(x, y).getUnit();
            if (unit !== null)
            {
                unit.setHasMoved(true);
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
                audio.addMusic("resources/music/cos/julia.ogg", 3969, 83932);
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

    this.superPowerStunChance = 1;

    this.powerStunChance = 0.5;
    this.powerOffBonus = 80;
    this.powerDefBonus = 10;

    this.d2dCoZoneOffBonus = 80;
    this.d2dCoZoneDefBonus = 10;

    this.d2dOffBonus = 65;
    this.d2dFixedDamage = true;

    this.getOffensiveBonus = function(co, attacker, atkPosX, atkPosY,
                                 defender, defPosX, defPosY, isDefender, action, luckmode, map)
    {
        if (CO.isActive(co))
        {
            var baseDamage = 0;
            var fixedDamage = false;
            if (map === null ||
                (map !== null && map.getGameRules().getCoGlobalD2D()))
            {
                baseDamage = CO_JULIA.d2dOffBonus;
                fixedDamage = CO_JULIA.d2dFixedDamage;
            }
            switch (co.getPowerMode())
            {
            case GameEnums.PowerMode_Tagpower:
            case GameEnums.PowerMode_Superpower:
            case GameEnums.PowerMode_Power:
                baseDamage = CO_JULIA.powerOffBonus;
                fixedDamage = true;
                break;
            default:
                if (co.inCORange(Qt.point(atkPosX, atkPosY), attacker))
                {
                    baseDamage = CO_JULIA.d2dCoZoneOffBonus;
                    fixedDamage = true;
                }
                break;
            }
            if (fixedDamage)
            {
                return baseDamage * 10 / attacker.getHpRounded() - 100;
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
                return CO_JULIA.powerDefBonus;
            }
            else if (co.inCORange(Qt.point(defPosX, defPosY), defender))
            {
                return CO_JULIA.d2dCoZoneDefBonus;
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
                return ["ZCOUNIT_PARTISAN"];
            }
        }
        return [];
    };

    // CO - Intel
    this.getBio = function(co)
    {
        return qsTr("A devoted CO who embraces Dark Matter's ideals and wishes to unite the world. She excels at utilizing propaganda to demoralize her enemies.");
    };
    this.getHits = function(co)
    {
        return qsTr("The morning paper");
    };
    this.getMiss = function(co)
    {
        return qsTr("The comics section");
    };
    this.getCODescription = function(co)
    {
        return qsTr("Units have reduced offensive power. However, firepower is unaffected by loss of HP.");
    };
    this.getLongCODescription = function(co, map)
    {
        var text = qsTr("\nSpecial Unit:\nPartisan\n");
        if (map === null ||
            (map !== null && map.getGameRules().getCoGlobalD2D()))
        {
            if (CO_JULIA.d2dFixedDamage)
            {

                text +=  qsTr("\nGlobal Effect: \nJulia's units have %1 firepower and their firepower is unaffected by loss of HP.");
            }
            else
            {
                text +=  qsTr("\nGlobal Effect: \nNone.");
            }
        }
        else
        {
            text +=  qsTr("\nGlobal Effect: \nNone.");
        }
        text += qsTr("\n\nCO Zone Effect: \nJulia's units have %0% firepower and +%2% defence. Their firepower is unaffected by loss of HP.");
        text = replaceTextArgs(text, [(CO_JULIA.d2dCoZoneOffBonus-100), CO_JULIA.d2dOffBonus, CO_JULIA.d2dCoZoneDefBonus]);
        return text;
    };
    this.getPowerDescription = function(co)
    {
        var text = qsTr("A random %0% of enemy units can't move next turn. Julia's units have %1% firepower and +%2% defence. Their firepower is unaffected by loss of HP.");
        text = replaceTextArgs(text, [CO_JULIA.powerStunChance * 100, (CO_JULIA.powerOffBonus-100), CO_JULIA.powerDefBonus]);
        return text;
    };
    this.getPowerName = function(co)
    {
        return qsTr("Rallying Cry");
    };
    this.getSuperPowerDescription = function(co)
    {
        var text = qsTr("%0% of enemy units can't move next turn. Julia's units have %1% firepower and +%2% defence. Their firepower is unaffected by loss of HP.");
        text = replaceTextArgs(text, [CO_JULIA.superPowerStunChance * 100, (CO_JULIA.powerOffBonus-100), CO_JULIA.powerDefBonus]);
        return text;
    };
    this.getSuperPowerName = function(co)
    {
        return qsTr("Media Mayhem");
    };
    this.getPowerSentences = function(co)
    {
        return [qsTr("Your defeat will be tomorrow's headline."),
                qsTr("Start the presses!"),
                qsTr("I don't receive the news. I make the news."),
                qsTr("I am the only voice of the people."),
                qsTr("Let us see how devoted your people are to your cause."),
                qsTr("You will come to embrace our ideals.")];
    };
    this.getVictorySentences = function(co)
    {
        return [qsTr("The proof of our might will forever be etched in your minds."),
                qsTr("How foolish it was of you to try to defy us."),
                qsTr("This victory will be pleasing news to our citizens.")];
    };
    this.getDefeatSentences = function(co)
    {
        return [qsTr("The news will still call this a victory for us."),
                qsTr("I will change the news...")];
    };
    this.getName = function()
    {
        return qsTr("Julia");
    };
}

Constructor.prototype = CO;
var CO_JULIA = new Constructor();
