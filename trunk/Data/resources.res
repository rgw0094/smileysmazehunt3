;***********************
;******** FONTS ********
;***********************
Font curlz {
 filename="curlz.fnt"
 tracking=1.0
 resgroup=3
}
Font description {
 filename="description.fnt"
 tracking=1.0
 resgroup=3
}
Font titleFnt {
 filename="titleFnt.fnt"
 tracking=1.0
 resgroup=3
}
Font controls {
 filename="controls.fnt"
 tracking=1.0
 resgroup=3
}
Font inventoryFnt {
 filename="inventory.fnt"
 tracking=1.0
 resgroup=3
}
Font numberFnt {
 filename="number.fnt"
 tracking=1.0
 resgroup=3
}
Font button {
 filename="buttons.fnt"
 tracking=1.0
 color=#000000
 resgroup=3
}
Font bigLoadFnt {
 filename="loadmenu1.fnt"
 tracking=1.0
 color=#000000
 resgroup=3
}
Font textBoxFnt {
 filename="textbox.fnt"
 tracking=1.0
 color=#000000
 resgroup=3
}
Font textBoxNameFnt {
 filename="textboxname.fnt"
 tracking=1.0
 color=#000000
 resgroup=3
}
Font textBoxDialogFnt {
 filename="textboxdialog.fnt"
 tracking=1.0
 color=#000000
 resgroup=3
}
Font newAreaFnt {
 filename="zone.fnt"
 tracking=1.0
 color=#000000
 resgroup=3
}

;***************************
;******** PARTICLES ********
;***************************
Texture particlesTx {
 filename="Graphics/particles.png"
 resgroup=2
}
Sprite particleGraphic1 {
 texture=particlesTx
 rect=0,0,32,32
 hotspot=16,16
 blendmode=COLORMUL,ALPHAADD,NOZWRITE
 resgroup=1
}
Sprite particleGraphic2 {
 texture=particlesTx
 rect=32,0,32,32
 hotspot=16,16
 blendmode=COLORMUL,ALPHAADD,NOZWRITE
 resgroup=1
}
Sprite particleGraphic3 {
 texture=particlesTx
 rect=64,0,32,32
 hotspot=16,16
 blendmode=COLORMUL,ALPHAADD,NOZWRITE
 resgroup=1
}
Sprite particleGraphic4 {
 texture=particlesTx
 rect=96,0,32,32
 hotspot=16,16
 blendmode=COLORMUL,ALPHAADD,NOZWRITE
 resgroup=1
}
Sprite particleGraphic5 {
 texture=particlesTx
 rect=0,32,32,32
 hotspot=16,16
 blendmode=COLORMUL,ALPHAADD,NOZWRITE
 resgroup=1
}
Sprite particleGraphic11 {
 texture=particlesTx
 rect=64,64,32,32
 hotspot=16,16
 blendmode=COLORMUL,ALPHAADD,NOZWRITE
 resgroup=1
}
Sprite particleGraphic13 {
 texture=particlesTx
 rect=0,96,32,32
 hotspot=16,16
 blendmode=COLORMUL,ALPHAADD,NOZWRITE
 resgroup=1
}
Sprite bloodSplatGraphic {
 texture=particlesTx
 rect=0,96,32,32
 hotspot=16,16
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=1
}
Particle enemyBlockCloud {
 filename=particle1.psi
 sprite=particleGraphic13
 resgroup=1
}
Particle sandCloud {
 filename=sandcloud.psi
 sprite=particleGraphic1
 resgroup=1
}
Particle iceNova {
 filename=icenova.psi
 sprite=particleGraphic13
 resgroup=1
}
Particle calypsoIceNova {
 filename=calypsoicenova.psi
 sprite=particleGraphic13
 resgroup=1
}
Particle treeletSpawn {
 filename=treelet.psi
 sprite=particleGraphic13
 resgroup=1
}
Particle penguinSplash {
 filename=penguinSplash.psi
 sprite=particleGraphic3
 resgroup=1
}
Particle bloodSplat {
 filename=bloodsplat.psi
 sprite=bloodSplatGraphic
 resgroup=1
}
Particle bombFuse {
 filename=bombfuse.psi
 sprite=particleGraphic2
 resgroup=1
}
Particle explosion {
 filename=explosion.psi
 sprite=particleGraphic11
 resgroup=1
}
Particle explosionLarge {
 filename=explosionLarge.psi
 sprite=particleGraphic11
 resgroup=1
}
Particle deathCloud {
 filename=deathCloud.psi
 sprite=particleGraphic1
 resgroup=1
}
Particle fireBall {
 filename=fireball.psi
 sprite=particleGraphic1
 resgroup=1
}
Particle fountain {
 filename=fountain.psi
 sprite=particleGraphic2
 resgroup=1
}
Particle iceOrb {
 filename=iceOrb.psi
 sprite=particleGraphic2
 resgroup=1
}
Particle fireOrb {
 filename=fireOrb.psi
 sprite=particleGraphic2
 resgroup=1
}
Particle smileysCane {
 filename=smileysCane.psi
 sprite=particleGraphic5
 resgroup=1
}
Particle flame {
 filename=flame.psi
 sprite=particleGraphic13
 resgroup=1
}

;***********************
;******** FENWAR *******
;***********************
Sprite fenwarDown {
 texture="Graphics/fenwar.png"
 rect=0,0,62,73
 hotspot=31,36
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=6
}


;*********************
;******** LOOT *******
;*********************
Sprite manaLoot {
 texture=general
 rect=0,160,30,30
 hotspot=15,15
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=7
}
Sprite healthLoot {
 texture=general
 rect=32,160,30,30
 hotspot=15,15
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=7
}
Sprite newAbilityLoot {
 texture=general
 rect=0,192,64,64
 hotspot=32,32
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=7
}

;***********************
;******** MUSIC ********
;***********************
Music menuMusic {
 filename="menu.xm"
 amplify=50
 resgroup=11
}
Music townMusic {
 filename="town.xm"
 amplify=60
 resgroup=11
}
Music bossMusic {
 filename="fireboss.xm"
 amplify=30
 resgroup=11
}
Music forestMusic {
 filename="forest.xm"
 amplify=60
 resgroup=11
}
Music iceMusic {
 filename="ice.xm"
 amplify=50
 resgroup=11
}
Music serpentinePathMusic {
 filename="serpentinePath.xm"
 amplify=50
 resgroup=11
}
Music oldeTowneMusic {
 filename="oldetowne.xm"
 amplify=70
 resgroup=11
}
Music smolderHollowMusic {
 filename="smolderhollow.xm"
 amplify=50
 resgroup=11
}
Music realmOfDespairMusic {
 filename="despair.xm"
 amplify=50
 resgroup=11
}
Music hintMusic {
 filename="hint.xm"
 amplify=90
 resgroup=11
}
Music kingTutMusic {
 filename="kingtut.mod"
 amplify=50
 resgroup=11
}

;**************************
;******** FOUNTAIN ********
;**************************
Animation fountainRipple {
 texture="Graphics/fountain.png"
 rect=0,0,338,95
 frames=10
 fps=10.0
 mode=FORWARD,LOOP
 hotspot=169,47.5
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=4
}
Sprite bigFountainTop {
 texture="Graphics/fountain.png"
 rect=0,380,340,128
 hotspot=170,120
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=4
}
Sprite bigFountainBottom {
 texture="Graphics/fountain.png"
 rect=0,508,340,44
 hotspot=170,-5
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=4
}
Sprite smallFountain {
 texture="Graphics/fountain.png"
 rect=340,380,118,173
 hotspot=59,120
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=4
}


;********************************
;******** TEXTBOX / SHOP ********
;********************************
Sprite textBox {
 texture=general
 rect=0,256,400,250
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=5
}
Sprite arrowIcon {
 texture=general
 rect=128,142,32,20
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=5
}
Sprite okIcon {
 texture=general
 rect=160,142,32,20
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=5
}
Sprite exitIcon {
 texture=general
 rect=206,64,40,40
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=5
}
Texture psychedelicTx {
 filename="Graphics/psychedelic.png"
 resgroup=5
}

;****************************
;******** ANIMATIONS ********
;****************************
Texture animations {
 filename="Graphics/animations.png"
 resgroup=2
}
Animation water {
 texture=animations
 rect=0,0,64,64
 frames=16
 fps=16.0
 mode=FORWARD,LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=2
}
Animation greenWater {
 texture=animations
 rect=0,640,64,64
 frames=16
 fps=16.0
 mode=FORWARD,LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=2
}
Animation lava {
 texture=animations
 rect=0,128,64,64
 frames=10
 fps=10.0
 mode=FORWARD,LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=2
}
Animation spring {
 texture=animations
 rect=0,64,64,64
 frames=7
 fps=14.0
 mode=FORWARD,LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=2
}
Animation superSpring {
 texture=animations
 rect=0,704,64,64
 frames=7
 fps=14.0
 mode=FORWARD,LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=2
}
Animation silverSwitch {
 texture=animations
 rect=320,192,64,64
 frames=5
 fps=20.0
 mode=FORWARD,LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=2
}
Animation brownSwitch {
 texture=animations
 rect=320,256,64,64
 frames=5
 fps=20.0
 mode=FORWARD,LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=2
}
Animation blueSwitch {
 texture=animations
 rect=320,320,64,64
 frames=5
 fps=20.0
 mode=FORWARD,LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=2
}
Animation greenSwitch {
 texture=animations
 rect=320,384,64,64
 frames=5
 fps=20.0
 mode=FORWARD,LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=2
}
Animation yellowSwitch {
 texture=animations
 rect=320,448,64,64
 frames=5
 fps=20.0
 mode=FORWARD,LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=2
}
Animation whiteSwitch {
 texture=animations
 rect=320,512,64,64
 frames=5
 fps=20.0
 mode=FORWARD,LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=2
}
Animation savePoint {
 texture=animations
 rect=0,576,64,64
 frames=16
 fps=16.0
 mode=FORWARD,LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=2
}

;***********************
;******** COMMON *******
;***********************
Sprite mouseCursor {
 texture=general
 rect=96,96,19,27
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=1
}
Sprite blackScreen {
 texture=general
 rect=97,65,30,30
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=1
}
Sprite reflectionShield {
 texture=general
 rect=0,64,96,96
 hotspot=48,48
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=1
}
Sprite iceBlock {
 texture=general
 rect=128,0,64,64
 hotspot=32,32
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=1
}
Sprite loading {
 texture="Graphics/loading.png"
 rect=1,1,400,400
 hotspot=200,200
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=1
}

;*********************
;******** MENU *******
;*********************
Texture menuBackgroundTx {
 filename="Graphics/titlescreen.png"
 resgroup=10
}
Texture menuItemsTx {
 filename="Graphics/menuitems.png"
 resgroup=10
}
Sprite menuBackground {
 texture=menuBackgroundTx
 rect=0,0,1024,768
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=10
}
Sprite menuSpeechBubble {
 texture=menuItemsTx
 rect=0,200,567,73
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=10
}
Sprite buttonBackground {
 texture=menuItemsTx
 rect=0,0,250,75
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=10
}
Sprite buttonBackgroundHighlighted {
 texture=menuItemsTx
 rect=250,0,250,75
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=10
}
Sprite menuSaveBox {
 texture=menuItemsTx
 rect=0,75,650,125
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=10
}
Sprite controlsBox {
 texture=menuItemsTx
 rect=0,273,210,30
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=10
}
Sprite selectedControlsBox {
 texture=menuItemsTx
 rect=0,303,210,30
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=10
}

;*************************
;******** MINI MAP *******
;*************************
Sprite mapBackground {
 texture="Graphics/mapborder.png"
 rect=0,0,660,492
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=11
}
Sprite mapFogOfWarLeft {
 texture=general
 rect=384,64,64,64
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=11
}
Sprite mapFogOfWarRight {
 texture=general
 rect=448,64,64,64
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=11
}
Sprite mapFogOfWarUp {
 texture=general
 rect=512,64,64,64
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=11
}
Sprite mapFogOfWarDown {
 texture=general
 rect=576,64,64,64
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=11
}
Sprite mapFogOfWarUpLeft {
 texture=general
 rect=640,64,32,32
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=11
}
Sprite mapFogOfWarUpRight {
 texture=general
 rect=672,64,32,32
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=11
}
Sprite mapFogOfWarDownRight {
 texture=general
 rect=672,96,32,32
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=11
}
Sprite mapFogOfWarDownLeft {
 texture=general
 rect=640,96,32,32
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=11
}


;********************
;******** GUI *******
;********************
Sprite abilityBox {
 texture="Graphics/gui.png"
 rect=0,30,85,85
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=25
}
Sprite manabarBackground {
 texture="Graphics/gui.png"
 rect=0,116,123,22
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=25
}
Sprite manaBar {
 texture="Graphics/gui.png"
 rect=0,138,115,16
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=25
}
Sprite moneyIcon {
 texture="Graphics/gui.png"
 rect=85,30,48,48
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=25
}
Sprite fullHealth {
 texture="Graphics/gui.png"
 rect=120,0,30,30
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=25
}
Sprite threeQuartersHealth {
 texture="Graphics/gui.png"
 rect=90,0,30,30
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=25
}
Sprite halfHealth {
 texture="Graphics/gui.png"
 rect=60,0,30,30
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=25
}
Sprite quarterHealth {
 texture="Graphics/gui.png"
 rect=30,0,30,30
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=25
}
Sprite emptyHealth {
 texture="Graphics/gui.png"
 rect=0,0,30,30
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=25
}
Sprite bossHealthBackground {
 texture="Graphics/gui.png"
 rect=0,154,256,43
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=25
}
Sprite bossHealthBar {
 texture="Graphics/gui.png"
 rect=0,197,230,32
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=25
}


;***********************
;******** PLAYER *******
;***********************
Sprite playerShadow {
 texture=general
 rect=128,162,64,15
 hotspot=32,7
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=4
}
Animation player {
 texture=general
 rect=1,507,61,72
 frames=8
 fps=0.0
 mode=LOOP
 hotspot=31,48
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=4
}
Animation smileyTongue {
 texture=animations
 rect=640,419,12,71
 frames=11
 fps=60.0
 mode=LOOP
 hotspot=6,70
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=4
}
Sprite smileysFace {
 texture=general
 rect=0,506,61,72
 hotspot=30,36
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=4
}

;**************************
;******** INVENTORY *******
;**************************
Texture InventoryTx {
 filename="Graphics/inventory.png"
 resgroup=20
}
Sprite inventoryCursor {
 texture=general
 rect=129,65,76,76
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=20
}
Sprite inventory {
 texture=InventoryTx
 rect=0,0,660,492
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=20
}
Animation levelIcons {
 texture=InventoryTx
 rect=0,492,40,40
 frames=5
 fps=20.0
 mode=LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=20
}
Animation keyIcons {
 texture=InventoryTx
 rect=200,492,40,40
 frames=5
 fps=20.0
 mode=LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=20
}
Animation upgradeIcons {
 texture=InventoryTx
 rect=360,492,40,40
 frames=5
 fps=20.0
 mode=LOOP
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=20
}

;***********************
;******** SOUNDS *******
;***********************
Sound snd_fireBreath {
 filename="FireBreath.wav"
 resgroup=30
}
Sound snd_splat {
 filename="splat.wav"
 resgroup=30
}
Sound snd_lava {
 filename="Lava.wav"
 resgroup=30
}
Sound snd_shallowWater {
 filename="ShallowWater.wav"
 resgroup=30
}
Sound snd_switch {
 filename="switch.wav"
 resgroup=30
}
Sound snd_spring {
 filename="spring.wav"
 resgroup=30
}
Sound snd_iceDie {
 filename="ice_die.wav"
 resgroup=30
}
Sound snd_warp {
 filename="warp.wav"
 resgroup=30
}
Sound snd_freeze {
 filename="freeze.wav"
 resgroup=30
}
Sound snd_key {
 filename="key.wav"
 resgroup=30
}
Sound snd_gem {
 filename="gem.wav"
 resgroup=30
}
Sound snd_iceBreath {
 filename="icebreath.wav"
 resgroup=30
}
Sound snd_drowning {
 filename="drowning.wav"
 resgroup=30
}
Sound snd_sillyPad {
 filename="sillypad.wav"
 resgroup=30
}
Sound snd_fireBossHit {
 filename="firebosshit.wav"
 resgroup=30
}
Sound snd_fireBossDie {
 filename="firebossdie.wav"
 resgroup=30
}
Sound snd_fireBossNova {
 filename="firebossnova.wav"
 resgroup=30
}
Sound snd_garmbornHit {
 filename="garmbornRoar.mp3"
 resgroup=30
}
Sound snd_treeletHit {
 filename="treeletRoar.wav"
 resgroup=30
}
Sound snd_enemyDeath {
 filename="enemyDeath.wav"
 resgroup=30
}
Sound snd_tongue {
 filename="tongue.wav"
 resgroup=30
}
Sound snd_penguinSplash {
 filename="penguinsplash.wav"
 resgroup=30
}
Sound snd_penguinStep {
 filename="penguinstep.wav"
 resgroup=30
}
Sound snd_fireWorm {
 filename="fireworm.wav"
 resgroup=30
}
Sound snd_purchaseUpgrade {
 filename="purchaseUpgrade.wav"
 resgroup=30
}

;****************************
;******** CORNWALLIS ********
;****************************
Texture CornwallisTx {
 filename="Graphics/cornwallis.png"
 resgroup=103
}
Sprite cornwallisBody {
 texture=CornwallisTx
 rect=0,0,107,158
 hotspot=53,78
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=103
}
Sprite redCornwallisBody {
 texture=CornwallisTx
 rect=107,0,107,158
 hotspot=53,78
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=103
}
Sprite spikeShadow {
 texture=CornwallisTx
 rect=0,158,20,20
 hotspot=10,10
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=103
}
Animation groundSpike {
 texture=CornwallisTx
 rect=20,158,20,60
 frames=5
 fps=20.0
 mode=LOOP
 hotspot=10,50
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=103
}

;*****************************
;****** LORD PHYREBOZZ *******
;*****************************
Texture phyrebozzTx {
 filename="Graphics/fireboss.png"
 resgroup=100
}
Animation phyrebozz {
 texture=phyrebozzTx
 rect=0,0,97,158
 frames=4
 fps=20.0
 mode=LOOP,PINGPONG
 hotspot=48,79
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=100
}
Animation phyrebozzDownMouth {
 texture=phyrebozzTx
 rect=0,158,32,17
 frames=4
 fps=12.0
 mode=NOLOOP,PINGPONG
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=100
}
Animation phyrebozzLeftMouth {
 texture=phyrebozzTx
 rect=0,175,27,13
 frames=4
 fps=12.0
 mode=NOLOOP,PINGPONG
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=100
}
Animation phyrebozzRightMouth {
 texture=phyrebozzTx
 rect=0,188,27,13
 frames=4
 fps=12.0
 mode=NOLOOP,PINGPONG
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=100
}

;*****************************
;****** PORTLY PENGUIN *******
;*****************************
Sprite penguinBody {
 texture="Graphics/penguin.png"
 rect=0,0,128,128
 hotspot=64,64
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=101
}

Sprite penguinSliding {
 texture="Graphics/penguin.png"
 rect=128,0,128,128
 hotspot=64,64
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=101
}

Sprite penguinDrowning {
 texture="Graphics/penguin.png"
 rect=256,0,128,128
 hotspot=64,64
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=101
}

Sprite penguinIceBlock {
 texture="Graphics/penguin.png"
 rect=48,128,64,64
 hotspot=0,0
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=101
}

;**************************
;******** MUSHBOOM ********
;**************************
Sprite mushboom {
 texture="Graphics/brian.png"
 rect=0,0,115,105
 hotspot=57,52
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=101
}
Sprite mushboomRightArm {
 texture="Graphics/brian.png"
 rect=0,105,39,25
 hotspot=0,0
 resgroup=101
}
Sprite mushboomLeftArm {
 texture="Graphics/brian.png"
 rect=0,131,39,25
 hotspot=0,24
 resgroup=101
}
Sprite mushboomBomb {
 texture="Graphics/brian.png"
 rect=0,156,35,46
 hotspot=18,29
 resgroup=101
}
Sprite mushboomBombShadow {
 texture="Graphics/brian.png"
 rect=36,190,35,12
 hotspot=18,-9
 resgroup=101
}

;**************************
;******** GARMBORN ********
;**************************
Texture garmbornTx {
 filename="Graphics/garmborn.png"
 resgroup=102
}
Sprite garmbornBody {
 texture=garmbornTx
 rect=0,0,192,192
 hotspot=96,96
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Sprite treelet {
 texture=garmbornTx
 rect=192,0,128,128
 hotspot=64,64
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Sprite grayTreelet {
 texture=garmbornTx
 rect=320,0,128,128
 hotspot=64,64
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Animation owlet {
 texture=garmbornTx
 rect=0,192,84,36
 frames=4
 fps=20.0
 mode=LOOP,PINGPONG
 hotspot=42,18
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}

;*****************************
;******** PROJECTILES ********
;*****************************
Texture projectileTx {
 filename="Graphics/projectiles.png"
 resgroup=2
}
Sprite basicProjectile {
 texture=projectileTx
 rect=0,0,20,20
 hotspot=10,10
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=36
}
Sprite frisbeeProjectile {
 texture=projectileTx
 rect=1,21,62,62
 hotspot=31,31
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=36
}
Sprite spikeProjectile {
 texture=projectileTx
 rect=0,85,64,14
 hotspot=32,7
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=36
}
Sprite lightningOrbProjectile {
 texture=projectileTx
 rect=0,99,64,31
 hotspot=16,16
 blendmode=COLORMUL,ALPHAADD,NOZWRITE
 resgroup=36
}
Sprite fishProjectile {
 texture="Graphics/penguin.png"
 rect=0,128,48,32
 hotspot=24,16
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=36
}
Sprite cannonballProjectile {
 texture=projectileTx
 rect=32,98,32,32
 hotspot=16,16
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=36
}
Sprite mushroomletProjectile {
 texture="Graphics/brian.png"
 rect=55,105,60,65
 hotspot=30,33
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=36
}
Sprite laserProjectile {
 texture="Graphics/projectiles.png"
 rect=64,0,5,130
 hotspot=2,65
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=36
}

;**************************
;******** Textures ********
;**************************
Texture itemLayer1 {
 filename="Graphics/itemlayer1.png"
 resgroup=2
}
Texture itemLayer2 {
 filename="Graphics/itemlayer2.png"
 resgroup=2
}
Texture general {
 filename="Graphics/sprites.png"
 resgroup=2
}
Texture enemies {
 filename="Graphics/enemies.png"
 resgroup=2
}
Texture tapestriesTx {
 filename="Graphics/tapestries.png"
}

;*************************
;******** Enemies ********
;*************************
Animation evileye {
 texture=animations
 rect=640,192,64,64
 frames=5
 fps=10.0
 mode=NOLOOP
 hotspot=32,32
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Animation gumdrop {
 texture=animations
 rect=640,256,64,64
 frames=5
 fps=10.0
 mode=NOLOOP
 hotspot=32,32
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Animation bombSpawn {
 texture=animations
 rect=448,64,64,64
 frames=8
 fps=10.0
 mode=NOLOOP
 hotspot=32,32
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Animation bombEyesGlow {
 texture=animations
 rect=640,128,64,64
 frames=6
 fps=20.0
 mode=PINGPONG
 hotspot=32,32
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Sprite bombRedCircle {
 texture=general
 rect=640,128,192,192
 hotspot=96,96
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=36
}
Animation crabWalk {
 texture=animations
 rect=640,320,64,64
 frames=2
 fps=3
 mode=LOOP
 hotspot=32,32
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Sprite clownChainDot {
 texture=animations
 rect=960,64,16,16
 hotspot=8,8
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Sprite clownHead {
 texture=animations
 rect=960,192,64,64
 hotspot=32,32
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Animation batlet {
 texture=animations
 rect=640,384,82,35
 frames=4
 fps=16.0
 mode=PINGPONG
 hotspot=32,32
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Sprite buzzardWing {
 texture=animations
 rect=768,320,98,27
 hotspot=100,14
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Sprite flailLink {
 texture=general
 rect=80,0,16,16
 hotspot=8,8
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Sprite flailHead {
 texture=general
 rect=0,0,64,64
 hotspot=32,32
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Sprite stunStar {
 texture=general
 rect=64,16,10,10
 hotspot=5,5
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Sprite evilWall {
 texture=general
 rect=256,64,64,64
 hotspot=32,32
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
Sprite evilWallSpike {
 texture=general
 rect=320,64,64,64
 hotspot=32,32
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=102
}
;************************
;******** CALYPSO *******
;************************
Texture CalypsoTx {
 filename="Graphics/calypso.png"
 resgroup=105
}
Sprite calypso {
 texture=CalypsoTx
 rect=0,0,160,160
 hotspot=80,80
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=105
}
Sprite evilCalypso {
 texture=CalypsoTx
 rect=0,160,160,160
 hotspot=80,80
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=105
}
Sprite calypsoShield {
 texture=CalypsoTx
 rect=160,0,180,180
 hotspot=90,90
 blendmode=COLORMUL,ALPHABLEND,NOZWRITE
 resgroup=105
}