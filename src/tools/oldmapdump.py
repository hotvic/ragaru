#!/bin/env python
#
# This file is part of Ragaru.
#
# Copyright © 2015 Victor A. Santos <victoraur.santos@gmail.com>
#
# Ragaru is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Ragaru is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Ragaru.  If not, see <http://www.gnu.org/licenses/>.


import sys, struct, argparse, ragmap


def dump_objects(m):
    print("Objects: ({})".format(m['objects'].numobjects))

    for i in range(m['objects'].numobjects):
        print("    → Type: {}".format(m['objects'].type[i]))
        print("    → Rotation: {}".format(m['objects'].rotation[i]))
        print("    → Rotation 2: {}".format(m['objects'].rotation2[i]))
        print("    → Scale: {}".format(m['objects'].scale[i]))
        print("    → Position: x({}) y({}) z({})".format(m['objects'].position[i].x, m['objects'].position[i].y, m['objects'].position[i].z))
        print("-" * 80)

def dump_hotspots(m):
    pass

def dump_players(m):
    # alias
    players = m['players']

    print("Players: ({})".format(m['numplayers']))

    for i in range(m['numplayers']):
        print("    → Coords: x({}) y({}) z({})".format(m['players'][i].coords.x, m['players'][i].coords.y, m['players'][i].coords.z))
        print("    → Rotation: {}".format(m['players'][i].rotation))
        print("    → Target Roatation: {}".format(m['players'][0].targetrotation))
        print("    → Armor: head({}) high({}) low({})".format(m['players'][i].armorhead, m['players'][i].armorhigh, m['players'][0].armorlow))
        print("    → Protection: head({}) high({}) low({})".format(m['players'][0].protectionhead, m['players'][0].protectionhigh, m['players'][0].protectionlow))
        print("    → Metal: head({}) high({}) low({})".format(m['players'][0].metalhead, m['players'][0].metalhigh, m['players'][0].metallow))
        print("    → Power: {}".format(m['players'][0].power))
        print("    → Speed Multiplier: {}".format(m['players'][0].speedmult))
        print("    → Appearence: skin({}) creature({})".format(m['players'][0].whichskin, m['players'][0].creature))
        print("    → Scale: {}".format(m['players'][0].scale))
        print("    → How Active: {}".format(m['players'][0].howactive))
        print("    → Immobile: {}".format(m['players'][0].immobile))
        print("    → Properties: head({}) body({}) armor({}) leg({})".format(m['players'][0].headprop, m['players'][0].bodyprop, m['players'][0].armprop, m['players'][0].legprop))

        print("\n    Clothes: ({})".format(players[i].numclothes))
        for j in range(players[i].numclothes):
            print("        → Tint: r({}) b({}) g({})".format(players[i].clothestint[j].r, players[i].clothestint[j].b, players[j].clothestint[j].g))
            print("        → {0}".format(players[i].clothes[j]))
            print("        ---")

        print("\n    Weapons: ({})".format(players[i].numweapons))
        for j in range(players[i].numweapons):
            print("    → Weapon {}".format(mplayers[i].weaponids[j]))

        print("\n    Waypoints: ({})".format(players[i].numwaypoints))
        for j in range(players[i].numwaypoints):
            print("        → Waypoint {}: x({}) y({}) z({})".format(players[i].waypointtype[j], players[i].waypoints[j].x, players[i].waypoints[j].y, players[i].waypoints[j].z))

        print("-" * 80)

def dump(m):
    print('''
version: {mapver}
indemo: {indemo}
type: {maptype}
hostile: {hostile}
environment: {environment}
viewdist: {viewdist}
fadestart: {fadestart}
mapradius: {mapradius}
Skybox - texture: {skyboxtexture}
Skybox - color: r: {skyboxr} | b: {skyboxb} | g: {skyboxg}
Skybox light - r:{slr} | b:{slb} | g: {slg}
mapcenter: x: {mapcenter.x} y:{mapcenter.y} z:{mapcenter.z}
    '''.strip().format(**m))

    print("\nDialogues: ({0})".format(m['numdialogues']))

    print("\nObjects: ({0}) [--objects to see]".format(m['objects'].numobjects))

    print("\nHotspots: ({0}) [--hotspots to see]".format(m['hotspots'].numhotspots))
    
    print("\nPlayers: ({0}) [--players to see all]".format(m['numplayers']))
    print('''
    → Coords: x({players[0].coords.x}) y({players[0].coords.y}) z({players[0].coords.z})
    → Rotation: {players[0].rotation}
    → Target Roatation: {players[0].targetrotation}
    → Armor: head({players[0].armorhead} high({players[0].armorhigh}) low({players[0].armorlow})
    → Protection: head({players[0].protectionhead}) high({players[0].protectionhigh}) low({players[0].protectionlow})
    → Metal: head({players[0].metalhead}) high({players[0].metalhigh}) low({players[0].metallow})
    → Power: {players[0].power}
    → Speed Multiplier: {players[0].speedmult}
    → Appearence: skin({players[0].whichskin}) creature({players[0].creature})
    → Scale: {players[0].scale}
    → How Active: {players[0].howactive}
    → Immobile: {players[0].immobile}
    → Properties: head({players[0].headprop}) body({players[0].bodyprop}) armor({players[0].armprop}) leg({players[0].legprop})
    '''[1:-1].format(**m))

    print("    → Clothes: ({0})".format(m['players'][0].numclothes))
    for i in range(m['players'][0].numclothes):
        print("        → Tint: r({}) b({}) g({})".format(m['players'][0].clothestint[i].r, m['players'][0].clothestint[i].b, m['players'][0].clothestint[i].g))
        print("        → {0}".format(m['players'][0].clothes[i]))
    
    print("    → Weapons: ({0})".format(m['players'][0].numweapons))
    for i in range(m['players'][0].numweapons):
        print("    → Weapon {0}".format(m['players'][0].weaponids[i]))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    parser.add_argument("map", help="map name to dump")
    parser.add_argument("-o", "--objects", action="store_true", help="display map objects")
    parser.add_argument("-H", "--hotspots", action="store_true", help="display map hotspots")
    parser.add_argument("-p", "--players", action="store_true", help="display all the players")

    args = parser.parse_args()

    map = ragmap.old_readmap(args.map)

    if (args.objects):
        dump_objects(map)
    elif (args.hotspots):
        dump_hotpots(map)
    elif (args.players):
        dump_players(map)
    else:
        dump(map)
