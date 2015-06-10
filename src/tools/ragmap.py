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


import struct


def old_readmap(mapname):
    with open(mapname, 'rb') as f:
        (mapver,) = struct.unpack(">i", f.read(4))
        (indemo,) = struct.unpack(">i", f.read(4)) if mapver >= 15 else (0, )
        (maptype,) = struct.unpack(">i", f.read(4)) if mapver >= 5 else (0, )
        (hostile,) = struct.unpack(">i", f.read(4)) if mapver >= 6 else (0, )
        (viewdist, fadestart) = struct.unpack(">ff", f.read(8)) if mapver >= 4 else (100., 0.6)
        (skyboxtexture, skyboxr, skyboxb, skyboxg) = struct.unpack(">bfff", f.read(13)) if mapver >= 2 else (True, 1., 1., 1.)
        (slr, slb, slg) = struct.unpack(">fff", f.read(12)) if mapver >= 10 else (skyboxr, skyboxb, skyboxg)
        mplayer = type('Player', (object,), {})
        mplayer.coords = type('XYZ', (object,), {})
        (mplayer.coords.x, mplayer.coords.y, mplayer.coords.z, mplayer.rotation, mplayer.targetrotation, mplayer.numweapons) = struct.unpack(">fffffi", f.read(6*4))
        (mplayer.armorhead, mplayer.armorhigh, mplayer.armorlow) = struct.unpack(">fff", f.read(4*3))
        (mplayer.protectionhead, mplayer.protectionhigh, mplayer.protectionlow) = struct.unpack(">fff", f.read(4*3))
        (mplayer.metalhead, mplayer.metalhigh, mplayer.metallow) = struct.unpack(">fff", f.read(4*3))
        (mplayer.power, mplayer.speedmult) = struct.unpack(">ff", f.read(4*2))
        (mplayer.numclothes, ) = struct.unpack(">i", f.read(4))

        #work around some bugged maps
        mplayer.numclothes = struct.unpack(">i", f.read(4))[0] if mplayer.numclothes > 5 else mplayer.numclothes

        (mplayer.whichskin, mplayer.creature) = struct.unpack(">ii", f.read(4*2)) if mapver >= 9 else (0, 0)

        numdialogueboxes = list()
        dialoguetype = list()
        participantlocation = list()
        participantrotation = list()
        dialogueboxlocation = list()
        dialogueboxcolor = list()
        dialogueboxsound = list()
        dialoguetext = list()
        dialoguename = list()
        dialoguecamera = list()
        participantfocus = list()
        participantaction = list()
        participantfacing = list()
        dialoguecamerarotation = list()
        dialoguecamerarotation2 = list()

        if mapver < 8: numdialogues = 0
        if mapver >= 8:
            (numdialogues, ) = struct.unpack(">i", f.read(4))
            if numdialogues:
                for i in range(numdialogues):
                    participantlocation.insert(i, list())
                    participantrotation.insert(i, list())
                    dialogueboxlocation.insert(i, list())
                    dialogueboxcolor.insert(i, list())
                    dialogueboxsound.insert(i, list())
                    dialoguetext.insert(i, list())
                    dialoguename.insert(i, list())
                    dialoguecamera.insert(i, list())
                    participantfocus.insert(i, list())
                    participantaction.insert(i, list())
                    participantfacing.insert(i, list())
                    dialoguecamerarotation.insert(i, list())
                    dialoguecamerarotation2.insert(i, list())

                    numdialogueboxes.insert(i, struct.unpack(">i", f.read(4))[0])
                    dialoguetype.insert(i, struct.unpack(">i", f.read(4))[0])

                    for j in range(10):
                        participantlocation[i].insert(j, type('', (object, ), {}))
                        (participantlocation[i][j].x, participantlocation[i][j].y, participantlocation[i][j].z) = struct.unpack(">fff", f.read(4*3))
                        participantrotation[i].insert(j, struct.unpack(">f", f.read(4))[0])

                    if numdialogueboxes:
                        for j in range(numdialogueboxes[i]):
                            dialoguetext[i].append(str())
                            dialoguename[i].append(str())
                            participantfacing[i].append(list())
                            dialogueboxcolor[i].insert(j, type('', (object, ), {}))
                            dialoguecamera[i].insert(j, type('', (object, ), {}))
                            dialogueboxlocation[i].insert(j, struct.unpack(">i", f.read(4)))
                            (dialogueboxcolor[i][j].r, dialogueboxcolor[i][j].b, dialogueboxcolor[i][j].g) = struct.unpack(">fff", f.read(4*3))
                            (dialogueboxsound[i], ) = struct.unpack(">i", f.read(4))

                            (tmplen, ) = struct.unpack(">i", f.read(4))
                            tmplen = 128 if tmplen > 128 or tmplen < 0 else tmplen
                            for k in range(tmplen):
                                tmpchar = struct.unpack(">c", f.read(1))[0].decode("utf-8")
                                dialoguetext[i][j] += tmpchar
                                if tmpchar == '\0': break

                            (tmplen, ) = struct.unpack(">i", f.read(4))
                            tmplen = 64 if tmplen > 64 or tmplen < 0 else tmplen
                            for k in range(tmplen):
                                tmpchar = struct.unpack(">c", f.read(1))[0].decode("utf-8")
                                dialoguename[i][j] += tmpchar
                                if tmpchar == '\0': break

                            (dialoguecamera[i][j].x, dialoguecamera[i][j].y, dialoguecamera[i][j].z) = struct.unpack(">fff", f.read(4*3))
                            participantfocus[i].insert(j, struct.unpack(">i", f.read(4)))
                            participantaction[i].insert(j, struct.unpack(">i", f.read(4)))

                            for k in range(10):
                                participantfacing[i][j].append(type('XYZ', (object, ), {}))
                                (participantfacing[i][j][k].x, participantfacing[i][j][k].y, participantfacing[i][j][k].z) = struct.unpack(">fff", f.read(4*3))

                            dialoguecamerarotation[i].insert(j, struct.unpack(">f", f.read(4)))
                            dialoguecamerarotation2[i].insert(j, struct.unpack(">f", f.read(4)))

        mplayer.clothes = list()
        mplayer.clothestint = list()
        if mplayer.numclothes:
            for i in range(mplayer.numclothes):
                mplayer.clothes.insert(i, str())
                mplayer.clothestint.insert(i, type('', (object, ), {}))

                (tmplen, ) = struct.unpack(">i", f.read(4))
                for j in range(tmplen):
                    tmpchar = struct.unpack(">c", f.read(1))[0].decode('utf-8')
                    mplayer.clothes[i] += tmpchar
                (mplayer.clothestint[i].r, mplayer.clothestint[i].b, mplayer.clothestint[i].g) = struct.unpack(">fff", f.read(4*3))

        (environment, ) = struct.unpack(">i", f.read(4))
        objects = type('Objects', (object, ), {})
        (objects.numobjects, ) = struct.unpack(">i", f.read(4))
        if objects.numobjects:
            objects.type = list()
            objects.rotation = list()
            objects.rotation2 = list()
            objects.position = list()
            objects.scale = list()

            for i in range(objects.numobjects):
                objects.type.append(None)
                objects.rotation.append(None)
                objects.rotation2.append(None)
                objects.position.insert(i, type('', (object, ), {}))
                objects.scale.append(None)
                (objects.type[i], objects.rotation[i], objects.rotation2[i], objects.position[i].x, objects.position[i].y, objects.position[i].z, objects.scale[i]) = struct.unpack(">iffffff", f.read(4*7))
                if objects.type[i] == 4:
                    objects.scale[i] = objects.scale[i - 1];

        hotspots = type('Hotspots', (object, ), {})
        if mapver >= 7:
            (hotspots.numhotspots, ) = struct.unpack(">i", f.read(4))
            if hotspots.numhotspots:
                hotspots.type = list()
                hotspots.size = list()
                hotspots.pos = list()
                hotspots.text = list()
                for i in range(hotspots.numhotspots):
                    hotspots.type.append(None)
                    hotspots.size.append(None)
                    hotspots.pos.append(type('XYZ', (object, ), {}))
                    hotspots.text.append(str())
                    (hotspots.type[i], hotspots.size[i], hotspots.pos[i].x, hotspots.pos[i].y, hotspots.pos[i].z) = struct.unpack(">iffff", f.read(4*5))

                    (tmplen, ) = struct.unpack(">i", f.read(4))
                    if tmplen:
                        for j in range(tmplen):
                            tmpchar = struct.unpack(">c", f.read(1))[0].decode('utf-8')
                            hotspots.text[i] += tmpchar

        players = list()
        (numplayers, ) = struct.unpack(">i", f.read(4))
        removeanother = False
        howmanyremoved = 0
        if numplayers > 0 and numplayers < 10:
            for i in range(numplayers - 1):
                index = i - howmanyremoved
                players.append(type('Player', (object, ), {}))
                players[index].coords = type('XYZ', (object, ), {})
                players[index].weaponsid = list()
                players[index].waypoints = list()
                players[index].waypointtype = list()

                (players[index].whichskin, players[index].creature, players[index].coords.x, players[index].coords.y, players[index].coords.z, players[index].numweapons) = struct.unpack(">iifffi", f.read(4*6))
                players[index].howactive = struct.unpack(">i", f.read(4))[0] if mapver >= 5 else 0
                players[index].scale = struct.unpack(">f", f.read(4))[0] if mapver >= 3 else -1
                players[index].immobile = struct.unpack(">b", f.read(1))[0] if mapver >= 11 else 0
                players[index].rotation = struct.unpack(">f", f.read(4))[0] if mapver >= 12 else 0
                players[index].targetrotation = players[index].rotation

                if numplayers < 0 and numplayers > 5:
                    removeanother = True
                    howmanyremoved += 1
                if not removeanother:
                    if players[index].numweapons > 0 and players[index].numweapons < 5:
                        for j in range(players[index].numweapons):
                            (tmpid, ) = struct.unpack(">i", f.read(4))
                            players[index].weaponsid.insert(j, tmpid)

                    (players[index].numwaypoints, ) = struct.unpack(">i", f.read(4))
                    for j in range(players[index].numwaypoints):
                        players[index].waypoints.append(type('XYZ', (object, ), {}))
                        players[index].waypointtype.append(None)

                        (players[index].waypoints[j].x, players[index].waypoints[j].y, players[index].waypoints[j].z) = struct.unpack(">fff", f.read(4*3))
                        players[index].waypointtype[j] = struct.unpack(">i", f.read(4))[0] if mapver >= 5 else 0

                    (players[index].waypoint, ) = struct.unpack(">i", f.read(4))
                    if players[index].waypoint > players[index].numwaypoints - 1: players[index].waypoint = 0

                    (players[index].armorhead, players[index].armorhigh, players[index].armorlow) = struct.unpack(">fff", f.read(4*3))
                    (players[index].protectionhead, players[index].protectionhigh, players[index].protectionlow) = struct.unpack(">fff", f.read(4*3))
                    (players[index].metalhead, players[index].metalhigh, players[index].metallow) = struct.unpack(">fff", f.read(4*3))
                    (players[index].power, players[index].speedmult) = struct.unpack(">ff", f.read(4*2))

                    (players[index].headprop, players[index].bodyprop, players[index].armprop, players[index].legprop) = struct.unpack(">ffff", f.read(4*4)) if mapver >= 4 else (1, 1, 1, 1)

                    players[index].numclothes = struct.unpack(">i", f.read(4))[0]
                    if players[index].numclothes:
                        players[index].clothes = list()
                        players[index].clothestint = list()

                        for j in range(players[index].numclothes):
                            players[index].clothes.append(str())
                            players[index].clothestint.append(type('RBG', (object, ), {}))

                            (tmplen, ) = struct.unpack(">i", f.read(4))
                            for k in range(tmplen):
                                tmpchar = struct.unpack(">c", f.read(1))[0]
                                players[index].clothes[j] += tmpchar

                            (players[index].clothestint[j].r, players[index].clothestint[j].b, players[index].clothestint[j].g) = struct.unpack(">fff", f.read(4*3))

        (numpathpoints, ) = struct.unpack(">i", f.read(4))
        pathpoint = list()
        numpathpointconnect = list()
        pathpointconnect = list()
        if numpathpoints:
            for i in range(numpathpoints):
                pathpoint.append(type('XYZ', (object, ), {}))
                numpathpointconnect.append(None)
                pathpointconnect.append(list())

                (pathpoint[i].x, pathpoint[i].y, pathpoint[i].z, numpathpointconnect[i]) = struct.unpack(">fffi", f.read(4*4))
                for j in range(numpathpointconnect[i]):
                    pathpointconnect[i].append(struct.unpack(">i", f.read(4))[0])

        mapcenter = type('XYZ', (object, ), {})
        (mapcenter.x, mapcenter.y, mapcenter.z, mapradius) = struct.unpack(">ffff", f.read(4*4))

    # mplayer at 0 and others at 1...
    tmpplayers = list()
    tmpplayers.append(mplayer)
    tmpplayers += players
    players = tmpplayers

    # some missing attr at main character
    players[0].scale = .0
    players[0].howactive = 0
    players[0].immobile = False
    (players[0].headprop, players[0].bodyprop, players[0].armprop, players[0].legprop) = (.0, .0, .0, .0)
    players[0].numwaypoints = 0

    return locals()