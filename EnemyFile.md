# Enemy Variables #

|Variable|Description|Possible Values|Mandatory|
|:-------|:----------|:--------------|:--------|
|OneGraphic|Uses same graphic for all directions|T/F            |N        |
|GCol    |Column the enemy graphics appear in enemies.png|int            |Y        |
|GRow    |Row the enemy graphics appear in enemies.png|int            |Y        |
|NumFrames|Number of 2-frame animations. If you omit this the default is 4|1-4            |N        |
|EnemyType|See Enemy Type table below|any int        |Y        |
|WanderType|See Wander Codes table|any int        |Y        |
|HP      |Health     |any int        |Y        |
|Damage  |Damage the enemy deals. 100 = 1 smiley face|any int        |Y        |
|Radius  |Radius of collision box|any int        |Y        |
|Speed   |Movement speed|any int        |Y        |
|Slime   |Can the enemy walk on slime?|T/F            |N        |
|Land    |Can the enemy walk on land?|T/F            |N        |
|Lava    |Can the enemy walk on lava?|T/F            |N        |
|Mushrooms|Can the enemy walk on mushrooms?|T/F            |N        |
|SWater  |Can the enemy walk on shallow water?|T/F            |N        |
|DWater  |Can the enemy walk on deep water?|T/F            |N        |
|ImmuneTongue|If the enemy is immune to Smiley's tongue|T/F            |N        |
|ImmuneFire|If the enemy is immune to fire breath|T/F            |N        |
|ImmuneLightning|If the enemy is immune to lightning|T/F            |N        |
|ImmuneStun|If the enemy is immune to stun|T/F            |N        |
|ImmuneFreeze|If the enemy is immune to freeze|T/F            |N        |
|Invincible|If the enemy is invincible|T/F            |N        |
|Variable 1|Usage is enemy-dependent, see below|any int        |N        |
|Variable 2|Usage is enemy-dependent, see below|any int        |N        |
|Variable 3|Usage is enemy-dependent, see below|any int        |N        |
|Chases  |If the enemy chases Smiley when he gets near|T/F            |If EnemyType==0|
|HasRanged|If the enemy has a ranged attack|T/F            |??       |
|Range   |Projectile attack range|any int        |If EnemyType==0 && HasRanged==T|
|Delay   |Projectile attack deley in milliseconds| any int       |If HasRanged==T|
|PDamage |Projectile damage. 100 = 1 smiley face|any int        |If HasRanged==T|
|PSpeed  |Projectile speed in pixels/second|any float      |If HasRanged==T|
|PType   |What type of projectile the enemy shoots|int, see chart below|If HasRanged==T|
|PHoming |Whether the projectiles home toward Smiley. Default is F|T/F            |N        |


---

# Enemy Types #
|Code|Enemy Type|Variable 1|Variable 2|Variable 3|
|:---|:---------|:---------|:---------|:---------|
|0   |Basic Enemy|          |          |
|1   |Evil eye  |          |          |
|2   |Burrower  |Animation N to use for burrowing<p>nomenclature to use in ResourceScript = 'burrowAnimN'<table><thead><th>Distance at which the burrower burrows (in pixels)</th></thead><tbody>
<tr><td>3   </td><td>Bomb generator</td><td>          </td><td>          </td></tr>
<tr><td>4   </td><td>Charger   </td><td>          </td><td>          </td></tr>
<tr><td>5   </td><td>Clown balloon</td><td>          </td><td>          </td></tr>
<tr><td>6   </td><td>Batlet distributor</td><td>          </td><td>          </td></tr>
<tr><td>7   </td><td>Buzzard   </td><td>          </td><td>          </td></tr>
<tr><td>8   </td><td>Sad Shooter</td><td>          </td><td>          </td></tr>
<tr><td>9   </td><td>Floater   </td><td>          </td><td>          </td></tr>
<tr><td>10  </td><td>Flailer   </td><td>          </td><td>          </td></tr>
<tr><td>11  </td><td>Tentacle  </td><td>Distance between nodes</td><td>          </td></tr>
<tr><td>12  </td><td>Turret    </td><td>0 for CW rotation, 1 for CCW, 2 for no movement</td><td>Direction (see below)</td></tr>
<tr><td>13  </td><td>Ghost     </td><td>          </td><td>          </td></tr>
<tr><td>14  </td><td>Fake Enemy</td><td>          </td><td>          </td></tr>
<tr><td>15  </td><td>Non-Gay Ranged Enemy</td><td>          </td><td>          </td></tr>
<tr><td>16  </td><td>Hopper    </td><td>          </td><td>          </td></tr>
<tr><td>17  </td><td>Spawner   </td><td>Enemy to spawn with 45% chance</td><td>Enemy to spawn with 35% chance</td><td>Enemy to spawn with 20% chance</td></tr>
<tr><td>18  </td><td>Adjacent shooter</td></tr>
<tr><td>19  </td><td>Botonoid  </td></tr>
<tr><td>20  </td><td>Diago Shooter (ex: Cone)</td></tr>
<tr><td>21  </td><td>Fenwar's Evil Spider-Eye</td></tr></tbody></table>

<hr />
<h1>Directions</h1>
<table><thead><th>Direction</th><th>Code</th></thead><tbody>
<tr><td>UP       </td><td>3   </td></tr>
<tr><td>DOWN     </td><td>0   </td></tr>
<tr><td>LEFT     </td><td>1   </td></tr>
<tr><td>RIGHT    </td><td>2   </td></tr></tbody></table>

<hr />
<h1>Wander Codes</h1>
<table><thead><th>Code</th><th>Description</th></thead><tbody>
<tr><td>0   </td><td>Wander around randomly</td></tr>
<tr><td>1   </td><td>Pace left and right</td></tr>
<tr><td>2   </td><td>Pace up and down</td></tr>
<tr><td>3   </td><td>Stand still</td></tr></tbody></table>

<hr />
<h1>Projectile Codes</h1>
<table><thead><th>Code</th><th>Description</th></thead><tbody>
<tr><td>0   </td><td>Blue ball  </td></tr>
<tr><td>1   </td><td>Frisbee    </td></tr>
<tr><td>2   </td><td>Lightning Orb</td></tr>
<tr><td>3   </td><td>Cactus spike</td></tr>
<tr><td>4   </td><td>Penguin fish</td></tr>
<tr><td>5   </td><td>Fireball   </td></tr>
<tr><td>6   </td><td>Turret cannonball</td></tr>
<tr><td>7   </td><td>Mini mushroom (shot by Mushboom)</td></tr>
<tr><td>8   </td><td>Laser (shot by Calypso)</td></tr>
<tr><td>9   </td><td>Tut's Lightning</td></tr>
<tr><td>10  </td><td>Tut's Mummy</td></tr>
<tr><td>11  </td><td>Candy (shot by Bartli)</td></tr>
<tr><td>12  </td><td>Figure 8-type</td></tr>
<tr><td>13  </td><td>Slime      </td></tr>
<tr><td>14  </td><td>Small orange dot</td></tr>
<tr><td>15  </td><td>Red boomerang shot by Botonoids</td></tr>
<tr><td>16  </td><td>Barvinoid's big white comet</td></tr>
<tr><td>17  </td><td>Barv's big yellow projectile</td></tr>
<tr><td>18  </td><td>Purple skull</td></tr>
<tr><td>19  </td><td>Acorn      </td></tr></tbody></table>

<hr />
<h1>Enemies by Area</h1>
<table><thead><th>Area</th><th>Enemies</th></thead><tbody>
<tr><td>Fountain</td><td>Red croc<sup>4</sup>, turtle<sup>4</sup></td></tr>
<tr><td>Olde Towne</td><td>Red croc<sup>4</sup>,Blue alien guy<sup>4</sup></td></tr>
<tr><td>Smolder Hollow</td><td>Fire Floater<sup>4</sup>, Fangie<sup>4</sup></td></tr>
<tr><td>Sessaria</td><td>Crystal<sup>4</sup>, Snowman<sup>4</sup>, Snowfang<sup>4</sup></td></tr>
<tr><td>Forest</td><td>Floating Turnip<sup>4</sup>, Squirrel<sup>4</sup>, Gumdrop<sup>4</sup></td></tr>
<tr><td>Desert</td><td>Buzzard<sup>4</sup>, Brown burrower<sup>4</sup>, Green Snake<sup>4</sup></td></tr>
<tr><td>Realm</td><td>Skullie<sup>4</sup>, Hopping Eye<sup>4</sup></td></tr>
<tr><td>Mush Cave</td><td>Beaker<sup>2</sup>, Cone<sup>2</sup></td></tr>
<tr><td>Pools</td><td>Orange head<sup>4</sup>, Friigoth Moorer<sup>4</sup></td></tr>
<tr><td>Mt Frostbite</td><td>Batlet<sup>4</sup>, Ice Cream Man<sup>4</sup>, Candy Floater<sup>4</sup>, Candy Spawner<sup>4</sup>, Sad Shooters<sup>4</sup></td></tr>
<tr><td>Swamp</td><td>Buzzard<sup>4</sup>, Tentacle<sup>4</sup>, Crab/Clown<sup>4</sup>, Pumpkin*3<sup>4</sup></td></tr>
<tr><td>Tut </td><td>Tan Snake<sup>4</sup>, Mummy<sup>4</sup></td></tr>
<tr><td>Castle</td><td>Proennok<sup>1</sup>,Ent<sup>3</sup>,Frownie<sup>1</sup>, Batlet(2)<sup>4</sup>, Spider<sup>3</sup>, Protector<sup>3</sup>,Green croc<sup>3</sup></td></tr>
<tr><td>Conservatory</td><td>Alphanoid<sup>2</sup>, Herbanoid<sup>2</sup>,Barvinoid<sup>2</sup></td></tr></tbody></table>

Superscript: 1 = not started, 2 = graphic done, 3 = put into Enemies.dat, 4 = put into level (done)