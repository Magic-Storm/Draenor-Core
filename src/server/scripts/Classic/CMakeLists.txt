

set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
  # Old* dungeon scripts omitted: same AddSC_* symbols as Kalimdor/EasternKingdoms
  # versions already registered in ScriptLoader.cpp (avoids LNK4006).

  Classic/Raids/OnyxiasLair/boss_onyxia_classic.cpp 
  Classic/Raids/OnyxiasLair/instance_onyxias_lair.cpp 
  Classic/Raids/OnyxiasLair/onyxias_lair.h
  
  
  Classic/Raids/BlackwingLair/boss_chromaggus.cpp
  Classic/Raids/BlackwingLair/boss_razorgore.cpp
  Classic/Raids/BlackwingLair/boss_firemaw.cpp
  Classic/Raids/BlackwingLair/boss_broodlord_lashlayer.cpp
  Classic/Raids/BlackwingLair/boss_ebonroc.cpp
  Classic/Raids/BlackwingLair/instance_blackwing_lair.cpp
  Classic/Raids/BlackwingLair/boss_vaelastrasz.cpp
  Classic/Raids/BlackwingLair/boss_nefarian.cpp
  Classic/Raids/BlackwingLair/boss_flamegor.cpp
  Classic/Raids/BlackwingLair/blackwing_lair.h
  
  
  Classic/Raids/ZulGurub/boss_hakkar.cpp
  Classic/Raids/ZulGurub/boss_mandokir.cpp
  Classic/Raids/ZulGurub/boss_marli.cpp
  Classic/Raids/ZulGurub/boss_hazzarah.cpp
  Classic/Raids/ZulGurub/boss_jeklik.cpp
  Classic/Raids/ZulGurub/boss_grilek.cpp
  Classic/Raids/ZulGurub/zulgurub.h
  Classic/Raids/ZulGurub/boss_renataki.cpp
  Classic/Raids/ZulGurub/boss_arlokk.cpp
  Classic/Raids/ZulGurub/boss_gahzranka.cpp
  Classic/Raids/ZulGurub/boss_venoxis.cpp
  Classic/Raids/ZulGurub/instance_zulgurub.cpp
  Classic/Raids/ZulGurub/boss_jindo.cpp
  Classic/Raids/ZulGurub/boss_wushoolay.cpp
  Classic/Raids/ZulGurub/boss_thekal.cpp
  
  Classic/Raids/MoltenCore/boss_gehennas.cpp
  Classic/Raids/MoltenCore/boss_lucifron.cpp
  Classic/Raids/MoltenCore/boss_golemagg.cpp
  Classic/Raids/MoltenCore/boss_majordomo_executus.cpp
  Classic/Raids/MoltenCore/boss_baron_geddon.cpp
  Classic/Raids/MoltenCore/boss_ragnaros.cpp
  Classic/Raids/MoltenCore/boss_garr.cpp
  Classic/Raids/MoltenCore/molten_core.h
  Classic/Raids/MoltenCore/instance_molten_core.cpp
  Classic/Raids/MoltenCore/boss_sulfuron_harbinger.cpp
  Classic/Raids/MoltenCore/boss_magmadar.cpp
  Classic/Raids/MoltenCore/boss_shazzrah.cpp
  
  
  Classic/World-Bosses/boss_azuregos.cpp
  Classic/World-Bosses/boss_classic_kazzak.cpp
  Classic/World-Bosses/boss_emerald_dragons.cpp

)

message("  -> Prepared: Classic")
