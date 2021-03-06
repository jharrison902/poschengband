#include "angband.h"

static cptr _desc = 
    "Golems are creatures animated by powerful magics: living stone conjured "
    "to serve their creators. But you have broken loose and are running amok!\n \n"
    "Golems are very slow when it comes to moving about, but this slowness does not "
    "affect their non-movement actions. Except for melee, where they are limited "
    "to just a single (but mighty) blow. They fight best with their powerful fists. "
    "Golems have a very high armor class, possess good resistances and saving throws, "
    "and are very strong in melee. They are also resistant to magic, and become "
    "more resistant as they evolve. This resistance is quite powerful, reducing the "
    "damage taken from all magical attacks.\n \n"
    "There are several varieties of golem, all sharing a common evolutionary "
    "heritage. First there is the mighty Colossus, the biggest (and slowest) of their "
    "kind. Next is the Sky Golem, a creature of powerful magic, resistant to the "
    "ravages of time. Finally, there is the Spellwarp Automaton, a creature of "
    "utter destruction, almost completely immune to magic. This last form takes "
    "nearly forever to build, unfortunately.";

static cptr _mon_name(int r_idx)
{
    if (r_idx)
        return r_name + r_info[r_idx].name;
    return ""; /* Birth Menu */
}

static int _rank(void)
{
    int r = 0;
    if (p_ptr->lev >= 10) r++;
    if (p_ptr->lev >= 20) r++;
    if (p_ptr->lev >= 30) r++;
    if (p_ptr->lev >= 40) r++;
    if (p_ptr->lev >= 45) r++;
    return r;
}

/**********************************************************************
 * Common Evolution: 
 *       10       20      30         40     45
 *  Clay -> Stone -> Iron -> Mithril -> Eog -> Colossus | Sky | Spellwarp
 **********************************************************************/
static void _birth(void) 
{ 
    object_type forge;

    p_ptr->current_r_idx = MON_CLAY_GOLEM;
    
    object_prep(&forge, lookup_kind(TV_HARD_ARMOR, SV_CHAIN_MAIL));
    add_outfit(&forge);

    object_prep(&forge, lookup_kind(TV_BOOTS, SV_PAIR_OF_METAL_SHOD_BOOTS));
    add_outfit(&forge);
}

static int _attack_level(void)
{
    int l = p_ptr->lev;
    switch (p_ptr->psubrace)
    {
    case GOLEM_COLOSSUS:
        l = MAX(1, l * 100 / 100);
        break;
    case GOLEM_SKY:
        l = MAX(1, l * 95 / 100);
        break;
    case GOLEM_SPELLWARP:
        l = MAX(1, l * 97 / 100);
        break;
    }
    return l;
}

static void _calc_innate_attacks(void)
{
    if (p_ptr->weapon_ct == 0 && equip_find_empty_hand())
    {
        innate_attack_t a = {0};
        int l = _attack_level();

        a.dd = 5 + l/5 + l*l/250;
        a.ds = 5 + l/5 + l*l/250;
        a.weight = 100 + 8*l;
        a.to_h = l;
        a.to_d = l + l*l/50 + l*l*l/2500;

        if (!equip_find_first(object_is_shield))
        {
            a.to_h += ((int)(adj_str_th[p_ptr->stat_ind[A_STR]]) - 128) + ((int)(adj_dex_th[p_ptr->stat_ind[A_DEX]]) - 128);
            a.to_d += ((int)(adj_str_td[p_ptr->stat_ind[A_STR]]) - 128) * 3/4;
        }

        if (l >= 30)
            a.effect[1] = GF_STUN;

        a.blows = 100;
        a.msg = "You punch %s.";
        a.name = "Fist";

        p_ptr->innate_attacks[p_ptr->innate_attack_ct++] = a;
    }
}

static void _gain_level(int new_level) 
{
    if (p_ptr->current_r_idx == MON_CLAY_GOLEM && new_level >= 10)
    {
        p_ptr->current_r_idx = MON_STONE_GOLEM;
        msg_print("You have evolved into a Stone Golem.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_STONE_GOLEM && new_level >= 20)
    {
        p_ptr->current_r_idx = MON_IRON_GOLEM;
        msg_print("You have evolved into an Iron Golem.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_IRON_GOLEM && new_level >= 30)
    {
        p_ptr->current_r_idx = MON_MITHRIL_GOLEM;
        msg_print("You have evolved into a Mithril Golem.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_MITHRIL_GOLEM && new_level >= 40)
    {
        p_ptr->current_r_idx = MON_EOG_GOLEM;
        msg_print("You have evolved into an Eog Golem.");
        p_ptr->redraw |= PR_MAP;
    }
    if (p_ptr->current_r_idx == MON_EOG_GOLEM && new_level >= 45)
    {
        switch (p_ptr->psubrace)
        {
        case GOLEM_COLOSSUS:
            p_ptr->current_r_idx = MON_COLOSSUS;
            msg_print("You have evolved into a Colossus.");
            break;
        case GOLEM_SKY:
            p_ptr->current_r_idx = MON_SKY_GOLEM;
            msg_print("You have evolved into a Sky Golem.");
            break;
        case GOLEM_SPELLWARP:
            p_ptr->current_r_idx = MON_SPELLWARP_AUTOMATON;
            msg_print("You have evolved into a Spellwarp Automaton.");
            break;
        }
        p_ptr->redraw |= PR_MAP;
    }
}

static void _calc_bonuses(void) 
{
    if (p_ptr->innate_attack_info.xtra_blow > 0)
        p_ptr->innate_attack_info.xtra_blow = 0;

    /* Clay Golem */
    p_ptr->to_a += 5;
    p_ptr->dis_to_a += 5;
    p_ptr->free_act = TRUE;
    p_ptr->hold_life = TRUE;
    res_add(RES_POIS);

    /* Stone Golem */
    if (p_ptr->lev >= 10)
    {
        p_ptr->to_a += 5;
        p_ptr->dis_to_a += 5;
        res_add(RES_FEAR);
    }

    /* Iron Golem */
    if (p_ptr->lev >= 20)
    {
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;
        p_ptr->see_inv = TRUE;
        res_add(RES_FIRE);
        res_add(RES_COLD);
        res_add(RES_ELEC);
        p_ptr->magic_resistance += 5;
    }

    /* Mithril Golem */
    if (p_ptr->lev >= 30)
    {
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;
        p_ptr->pspeed -= 1;
        p_ptr->reflect = TRUE;
        res_add(RES_CONF);
        res_add(RES_SHARDS);
        p_ptr->magic_resistance += 5;
    }

    /* Eog Golem */
    if (p_ptr->lev >= 40)
    {
        p_ptr->to_a += 10;            /* +40 */
        p_ptr->dis_to_a += 10;
        p_ptr->pspeed -= 1;
        p_ptr->magic_resistance += 5; /* 15% */
    }

    if (p_ptr->current_r_idx == MON_COLOSSUS)
    {
        res_add(RES_SOUND);
        res_add(RES_DISEN);
        p_ptr->pspeed -= 3;
        p_ptr->to_a += 35;
        p_ptr->dis_to_a += 35;
        p_ptr->magic_resistance += 5;
    }

    if (p_ptr->current_r_idx == MON_SKY_GOLEM)
    {
        res_add(RES_COLD);
        res_add(RES_TIME);
        res_add(RES_LITE);
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;
        p_ptr->magic_resistance += 5;
    }

    if (p_ptr->current_r_idx == MON_SPELLWARP_AUTOMATON)
    {
        res_add(RES_TELEPORT);
        p_ptr->pspeed -= 1;
        p_ptr->no_stun = TRUE;
        p_ptr->to_a += 10;
        p_ptr->dis_to_a += 10;
        p_ptr->magic_resistance += 20;
    }
}

static void _get_flags(u32b flgs[TR_FLAG_SIZE]) 
{
    /* Clay Golem */
    add_flag(flgs, TR_FREE_ACT);
    add_flag(flgs, TR_HOLD_LIFE);
    add_flag(flgs, TR_RES_POIS);

    /* Stone Golem */
    if (p_ptr->lev >= 10)
    {
        add_flag(flgs, TR_RES_FEAR);
    }

    /* Iron Golem */
    if (p_ptr->lev >= 20)
    {
        add_flag(flgs, TR_SEE_INVIS);
        add_flag(flgs, TR_RES_FIRE);
        add_flag(flgs, TR_RES_COLD);
        add_flag(flgs, TR_RES_ELEC);
        add_flag(flgs, TR_MAGIC_RESISTANCE);
    }

    /* Mithril Golem */
    if (p_ptr->lev >= 30)
    {
        add_flag(flgs, TR_SPEED);
        add_flag(flgs, TR_RES_CONF);
        add_flag(flgs, TR_RES_SHARDS);
        add_flag(flgs, TR_REFLECT);
    }

    /* Eog Golem */
    if (p_ptr->lev >= 40)
    {
    }

    if (p_ptr->current_r_idx == MON_COLOSSUS)
    {
        add_flag(flgs, TR_RES_SOUND);
        add_flag(flgs, TR_RES_DISEN);
    }

    if (p_ptr->current_r_idx == MON_SKY_GOLEM)
    {
        add_flag(flgs, TR_RES_TIME);
        add_flag(flgs, TR_RES_COLD);
        add_flag(flgs, TR_RES_LITE);
    }

    if (p_ptr->current_r_idx == MON_SPELLWARP_AUTOMATON)
    {
    }
}

/**********************************************************************
 * Powers
 **********************************************************************/
static void _big_punch_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Big Punch");
        break;
    case SPELL_DESC:
        var_set_string(res, "Attack an adjacent opponent with a single devastating blow.");
        break;
    case SPELL_CAST:
        if (p_ptr->innate_attack_ct)
            var_set_bool(res, do_blow(GOLEM_BIG_PUNCH));
        else
        {
            msg_print("Not while you are wielding a weapon!");
            var_set_bool(res, FALSE);
        }
        break;
    default:
        default_spell(cmd, res);
        break;
    }
}
void _breathe_cold_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Cold");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes cold at chosen target");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, p_ptr->chp / 2));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;

        msg_print("You breathe cold.");
        fire_ball(GF_COLD, dir, p_ptr->chp / 2, -3);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _breathe_disintegration_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Disintegration");
        break;
    case SPELL_DESC:
        var_set_string(res, "A disintegration breath.  Not even the dungeon walls can withstand its power!");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, p_ptr->chp / 4));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;

        msg_print("You breathe disintegration.");
        fire_ball(GF_DISINTEGRATE, dir, p_ptr->chp / 4, -3);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _breathe_light_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Light");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes light at chosen target");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, p_ptr->chp / 3));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;

        msg_print("You breathe light.");
        fire_ball(GF_LITE, dir, p_ptr->chp / 3, -3);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _breathe_time_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Breathe Time");
        break;
    case SPELL_DESC:
        var_set_string(res, "Breathes time at chosen target");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(0, 0, p_ptr->chp / 5));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;

        msg_print("You breathe time.");
        fire_ball(GF_TIME, dir, p_ptr->chp / 5, -3);

        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
void _shoot_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Shoot");
        break;
    case SPELL_DESC:
        var_set_string(res, "Shoot a missile at chosen target.");
        break;
    case SPELL_INFO:
        var_set_string(res, info_damage(15, 15, 0));
        break;
    case SPELL_CAST:
    {
        int dir = 0;
        var_set_bool(res, FALSE);
        if (!get_aim_dir(&dir)) return;
        msg_print("You shoot a missile.");
        fire_bolt(GF_MISSILE, dir, damroll(15, 15));
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static void _stone_smash_spell(int cmd, variant *res)
{
    switch (cmd)
    {
    case SPELL_NAME:
        var_set_string(res, "Stone Smash");
        break;
    case SPELL_DESC:
        var_set_string(res, "Destroys adjacent targeted wall.");
        break;
    case SPELL_CAST:
    {
        int y, x, dir;
        
        var_set_bool(res, FALSE);
        if (!get_rep_dir2(&dir)) return;
        if (dir == 5) return;

        y = py + ddy[dir];
        x = px + ddx[dir];
        
        if (!in_bounds(y, x)) return;

        if (cave_have_flag_bold(y, x, FF_HURT_ROCK))
        {
            cave_alter_feat(y, x, FF_HURT_ROCK);
            p_ptr->update |= PU_FLOW;
        }
        var_set_bool(res, TRUE);
        break;
    }
    default:
        default_spell(cmd, res);
        break;
    }
}
static power_info _powers[] = 
{
    { A_STR, {  5,  5, 30, _stone_smash_spell} },
    { A_CON, { 10, 10, 30, stone_skin_spell} },
    { A_STR, { 40,100,  0, _big_punch_spell} },
    {    -1, { -1, -1, -1, NULL} }
};
static power_info _colossus_powers[] = 
{
    { A_STR, { 45, 15, 50, _shoot_spell} },
    {    -1, { -1, -1, -1, NULL} }
};
static power_info _sky_powers[] = 
{
    { A_STR, { 45, 10, 50, ice_bolt_spell} },
    { A_CON, { 45, 30, 50, _breathe_light_spell} },
    { A_CON, { 45, 30, 50, _breathe_cold_spell} },
    { A_CON, { 50, 35, 60, _breathe_time_spell} },
    {    -1, { -1, -1, -1, NULL} }
};
static power_info _spellwarp_powers[] = 
{
    { A_STR, { 45, 25, 50, _breathe_disintegration_spell} },
    {    -1, { -1, -1, -1, NULL} }
};
static int _get_powers(spell_info* spells, int max) 
{
    int ct = get_powers_aux(spells, max, _powers);
    switch (p_ptr->current_r_idx)
    {
    case MON_COLOSSUS:
        ct += get_powers_aux(spells + ct, max - ct, _colossus_powers);
        break;
    case MON_SKY_GOLEM:
        ct += get_powers_aux(spells + ct, max - ct, _sky_powers);
        break;
    case MON_SPELLWARP_AUTOMATON:
        ct += get_powers_aux(spells + ct, max - ct, _spellwarp_powers);
        break;
    }

    return ct;
}

/**********************************************************************
 * Public
 **********************************************************************/
race_t *mon_golem_get_race_t(int psubrace)
{
    static race_t me = {0};
    static bool   init = FALSE;
    int           rank = _rank();

    if (!init)
    {           /* dis, dev, sav, stl, srh, fos, thn, thb */
    skills_t bs = { 25,  18,  40,   0,  10,   7,  70,  30};
    skills_t xs = { 10,   7,  15,   0,   0,   0,  30,  10};

        me.skills = bs;
        me.extra_skills = xs;

        me.infra = 5;
        init = TRUE;

        me.name = "Golem";
        me.desc = _desc;
        me.calc_innate_attacks = _calc_innate_attacks;
        me.birth = _birth;
        me.gain_level = _gain_level;
        me.calc_bonuses = _calc_bonuses;
        me.get_powers = _get_powers;
        me.get_flags = _get_flags;
        me.flags = RACE_IS_MONSTER | RACE_IS_NONLIVING;
        me.base_hp = 50;
        me.boss_r_idx = MON_DESTROYER;
        me.pseudo_class_idx = CLASS_MAULER;
    }

    me.subname = _mon_name(p_ptr->current_r_idx);

    me.stats[A_STR] =  1 + rank;
    me.stats[A_INT] = -2 - (rank+1)/2;
    me.stats[A_WIS] = -2 - (rank+1)/2;
    me.stats[A_DEX] = -1 - (rank+1)/2;
    me.stats[A_CON] =  1 + rank;
    me.stats[A_CHR] =  0 + (rank+1)/2;

    switch (psubrace)
    {
    case GOLEM_SKY:
        if (!p_ptr->current_r_idx)
            me.subname = "Sky Golem";
        me.life = 100 + 2*rank;
        me.exp = 250;
        break;

    case GOLEM_SPELLWARP:
        if (!p_ptr->current_r_idx)
            me.subname = "Spellwarp Automaton";
        me.life = 100 + 3*rank;
        me.exp = 350;
        break;

    case GOLEM_COLOSSUS:
    default:
        if (!p_ptr->current_r_idx)
            me.subname = "Colossus";
        if (p_ptr->current_r_idx == MON_COLOSSUS)
        {
            me.stats[A_STR] += 3;
            me.stats[A_DEX] -= 2;
            me.stats[A_CON] += 3;
        }
        me.life = 100 + 5*rank;
        me.exp = 200;
        break;
    }


    return &me;
}

