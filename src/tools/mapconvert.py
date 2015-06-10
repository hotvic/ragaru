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


import sys, struct, re, ragmap


def fixclothesname(clothes):
    c = re.split(':', clothes)

    return '/'.join(c[2:]).lower()

def writemap(mapname, m):
    with open(mapname, 'wb') as f:
        f.write(struct.pack('>i', m['mapver']))
        f.write(struct.pack('>i', m['indemo']))
        f.write(struct.pack('>i', m['maptype']))
        f.write(struct.pack('>i', m['hostile']))
        f.write(struct.pack('>i', m['environment']))
        f.write(struct.pack('>f', m['viewdist']))
        f.write(struct.pack('>f', m['fadestart']))
        f.write(struct.pack('>f', m['mapradius']))
        f.write(struct.pack('>i', m['skyboxtexture']))
        f.write(struct.pack('>fff', m['skyboxr'], m['skyboxb'], m['skyboxg']))
        f.write(struct.pack('>fff', m['slr'], m['slb'], m['slg']))
        f.write(struct.pack('>fff', m['mapcenter'].x, m['mapcenter'].y, m['mapcenter'].z))

        f.write(struct.pack('>i', m['numdialogues']))
        if m['numdialogues']:
            for i in range(m['numdialogues']):
                f.write(struct.pack('>i', m['dialoguetype'][i]))

                for j in range(10):
                    f.write(struct.pack('>f', m['participantrotation'][i][j]))
                    f.write(struct.pack('>fff', m['participantlocation'][i][j].x, m['participantlocation'][i][j].y, m['participantlocation'][i][j].z))

                f.write(struct.pack('>i', m['numdialogueboxes'][i]))
                if m['numdialogueboxes'][i]:
                    for j in range(m['numdialogueboxes'][i]):
                        f.write(struct.pack('>i', m['dialogueboxlocation'][i][j]))
                        f.write(struct.pack('>i', m['dialogueboxsound'][i][j]))
                        f.write(struct.pack('>fff', m['dialogueboxcolor'][i][j].r, m['dialogueboxcolor'][i][j].b, m['dialogueboxcolor'][i][j].g))
                        f.write(struct.pack('>fff', m['dialoguecamera'][i][j].x, m['dialoguecamera'][i][j].y, m['dialoguecamera'][i][j].z))
                        f.write(struct.pack('>f', m['dialoguecamerarotation'][i][j]))
                        f.write(struct.pack('>f', m['dialoguecamerarotation2'][i][j]))
                        f.write(struct.pack('>i', m['participantfocus'][i][j]))
                        f.write(struct.pack('>i', m['participantaction'][i][j]))

                        for c in m['dialoguename'][i][j]:
                            f.write(struct.pack('>c', c))
                        f.write(struct.pack('>c', '\0'))

                        for c in m['dialoguetext'][i][j]:
                            f.write(struct.pack('>c', c))
                        f.write(struct.pack('>c', '\0'))

                        for k in range(10):
                            f.write(struct.pack('>fff', m['participantfacing'][i][j][k].x,  m['participantfacing'][i][j][k].y,  m['participantfacing'][i][j][k].z))

        f.write(struct.pack('>i', m['objects'].numobjects))
        if m['objects'].numobjects:
            for i in range(m['objects'].numobjects):
                f.write(struct.pack('>i', m['objects'].type[i]))
                f.write(struct.pack('>f', m['objects'].rotation[i]))
                f.write(struct.pack('>f', m['objects'].rotation2[i]))
                f.write(struct.pack('>f', m['objects'].scale[i]))
                f.write(struct.pack('>fff', m['objects'].position[i].x, m['objects'].position[i].y, m['objects'].position[i].z))

        f.write(struct.pack('>i', m['hotspots'].numhotspots))
        if m['hotspots'].numhotspots:
            for i in range(m['hotspots'].numhotspots):
                f.write(struct.pack('>i', m['hotspots'].type[i]))
                f.write(struct.pack('>f', m['hotspots'].size[i]))
                f.write(struct.pack('>fff', m['hotspots'].pos[i].x, m['hotspots'].pos[i].y, m['hotspots'].pos[i].z))

                for c in m['hotspots'].text[i]:
                    f.write(struct.pack('>c', c))
                f.write(struct.pack('>c', '\0'))

        f.write(struct.pack('>i', m['numpathpoints']))
        if m['numpathpoints']:
            for i in range(m['numpathpoints']):
                f.write(struct.pack('>fff', m['pathpoint'][i].x, m['pathpoint'][i].y, m['pathpoint'][i].z))

                f.write(struct.pack('>i', m['numpathpointconnect'][i]))
                for j in range(m['numpathpointconnect'][i]):
                    f.write(struct.pack('>i', m['pathpointconnect'][i][j]))

        f.write(struct.pack('>i', len(m['players'])))
        for i, player in enumerate(m['players']):
            f.write(struct.pack('>fff', player.coords.x, player.coords.y, player.coords.z))
            f.write(struct.pack('>f', player.rotation))
            f.write(struct.pack('>f', player.targetrotation))
            f.write(struct.pack('>fff', player.armorhead, player.armorhigh, player.armorlow))
            f.write(struct.pack('>fff', player.protectionhead, player.protectionhigh, player.protectionlow))
            f.write(struct.pack('>fff', player.metalhead, player.metalhigh, player.metallow))
            f.write(struct.pack('>ff', player.power, player.speedmult))
            f.write(struct.pack('>ii', player.whichskin, player.creature))
            f.write(struct.pack('>f', player.scale))
            f.write(struct.pack('>i', player.howactive))
            f.write(struct.pack('>b', player.immobile))
            f.write(struct.pack('>ffff', player.headprop, player.bodyprop, player.armprop, player.legprop))

            f.write(struct.pack('>i', player.numclothes))
            for j in range(player.numclothes):
                f.write(struct.pack('>fff', player.clothestint[j].r, player.clothestint[j].b, player.clothestint[j].g))

                clothes = fixclothesname(player.clothes[j])
                for c in clothes:
                    f.write(struct.pack('>c', c))
                f.write(struct.pack('>c', '\0'))

            f.write(struct.pack('>i', player.numweapons))
            for j in range(player.numweapons):
                f.write(struct.pack('>i', player.weaponsid[j]))

            f.write(struct.pack('>i', player.numwaypoints))
            for j in range(player.numwaypoints):
                f.write(struct.pack('>i', player.waypointtype[j]))
                f.write(struct.pack('>fff', player.waypoints[j].x, player.waypoints[j].y, player.waypoints[j].z))


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("usage: {0} <orig_name> <dest_name>".format(sys.argv[0]))
        sys.exit(1)

    r = ragmap.old_readmap(sys.argv[1])
    writemap(sys.argv[2], r)

    for i in range(r["mplayer"].numclothes):
        fixclothesname(r["mplayer"].clothes[i])
