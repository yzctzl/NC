# navicat-keygen for windows - How to use?

[中文版](how-to-use.windows.zh-CN.md)

1. Use `navicat-patcher.exe` to replace __Navicat Activation Public Key__ that is stored in `libcc.dll`.
   
   ```
   navicat-patcher.exe [-dry-run] <Navicat Install Path> [RSA-2048 PEM File Path]
   ```

   * `[-dry-run]` Run patcher without applying any patches.

     __This parameter is optional.__ 

   * `<Navicat Install Path>`: The full path to Navicat installation folder. 
     
     __This parameter must be specified.__

   * `[RSA-2048 PEM File Path]`: The full path or relative path to a RSA-2048 private key file. 
     
     __This parameter is optional.__ If not specified, `navicat-patcher.exe` will generate a new RSA-2048 private key file `RegPrivateKey.pem` at current directory.

   __Example: (in cmd.exe)__ 

   ```
   navicat-patcher.exe "C:\Program Files\PremiumSoft\Navicat Premium 16"
   ```
   
   It has been tested on __Navicat Premium 16.0.7 English version__. The following is an example of output.

   ```
   ***************************************************
   *       navicat-patcher by @DoubleLabyrinth       *
   *               version: 16.0.7.0                 *
   ***************************************************

   [+] Try to open libcc.dll ... OK!

   [*] patch_solution_since<16, 0, 7, 0>: m_va_CSRegistrationInfoFetcher_WIN_vtable = 0x00000001837759f0
   [*] patch_solution_since<16, 0, 7, 0>: m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey = 0x0000000181fa52d0
   [*] patch_solution_since<16, 0, 7, 0>: m_va_iat_entry_malloc = 0x0000000183439bd0
   [+] patch_solution_since<16, 0, 7, 0>: official encoded key is found.

   [*] Generating new RSA private key, it may take a long time...
   [*] Your RSA private key:
   -----BEGIN RSA PRIVATE KEY-----
   MIIEpQIBAAKCAQEAvxaFFjTE6hi80nhjgfFMM3yPer122OIWIbbumFIuAOcCF6D3
   PnRHBdP9IqB99K6Nv6vKK3Jf0Y+dc5ETrg0l0AHYq+dTFTiWusHuRMx6xFjWzO96
   7mFmJq6P28dUucKnr6yG1TQeZaq+mHh2DNEnNEYgV7cLVT1unUmMOL/PBh/eCcaJ
   8hQNTQafQQknzCnAKC89v33y+rKInJNy9B+zSB0BGCz+eS8MKf6zc78JMSOnF2uj
   NK+QEwaYw8lAbJve1F+rCQS0mbm0QvHhZYZrblVHI5l/8LkX5qBtKw7duUhXHxmO
   fQieF23bBk9HDp5uQUGsdbKX6ZWitn/h926xyQIDAQABAoIBAQCHXxDRdni5zuSV
   xivYdnUhVHDg5zA23ZQINmw5BJ8KjJzy2FnPqNhXzKJb0Y7ptG8/BhinRtOSxkcp
   A/IJL89F2MkCn1JAimJd091UZ/fg+X7SmCVikyWm6auIa2IeZ0QcNAEhMVcHdzqn
   EU+wLMu1QKjQ+x/QN0ERtHTeDyQ+lUNB+bvAjx3LHN9Zh8weVBHHtwDoyyZDdJPw
   NWgpgcW+uYzlT66uh7LPPaRsEZgAkPIkhzZnwmugXdhlWxtYHKTEfe5gCqubQICc
   I/x1yBP1EZFm6qBQD4/49775ZbXwxgaWvBXG+Aah9x8JYtVUS4MgrAiC4a8NQqFp
   nwKVjUIBAoGBAOWsj9GGb2KYbfLzJNRrSxhs4TUBfpHteKSm2pL92NAbIOjssNhL
   hLY3gBFX2RnYmoGD6YT84JNykuAictgAd5GwvLIbaVF9l7MQn8APRbe2CzQ+/494
   9hpn33MZOBNd3I+a5+2qoFbXI04loyYDJkkeOqbwZzJjs7k9HmZMNwY5AoGBANT9
   tRFWFDvA0pPgGoHhzlsAUAmrbSfCPkhrRXpE9fgl3VnV+NRtjCf9NhJt0uaIokZ5
   oSf+jClcwU8N4EvGxMBaCHTqBzgc4dLPWpMAhPoMjjv1Oyug2iBcuTasHVP+Jdgq
   CaNzpXOuq4upaaNrq+QMsI6O9wA/zWhWPmnYQYgRAoGAUk56471noU+65zvXUQB6
   UvCB7Hrynt0ZRPg+kDrEPh/atV5NKdY2Yw6UqKJwvOBwzkU1pGDzIiQHGqd9vIa+
   Usmhdbp5DakSeitU9IEEnQdyEHEbKJFSsLfUzeyVuesDJbt/rh5dg4Fpt5GpW+/5
   Am8A2d6BPP+Z4qJSiJp7hZECgYEAy64TCZEXqEytE1yr/KjDfaK+54BX0j2e8gIj
   XtmznqoXE2Hboslfzp4Gp3j+xhbDmEGYK3bw8l0RP1g1tkFOxeNTUvq6DJ8SFVbV
   dt54S+bV3eCVxRL9hRUmyXGuWjQgXKdWsEhXYFkZE2Xe77h3mI3KCYoOCt74v146
   MV3szQECgYEAozTO7Wuum+VMKIY35hmHMjUiYmLl3EXWwMBT2VSsk8Siu0XoH0yd
   KoxsLDUBMS8sWKCZhFwU+Fx8UZjfo+xE3H4UTyVsw5EDpB9gSud928gNADwxTKor
   3s4jnUzb4XRQ0qN2jXzdNuqXNV1ozeqajbM2oSZqbSnWSs5g6DpIs1Q=
   -----END RSA PRIVATE KEY-----

   [*] patch_solution_since<16, 0, 7, 0>: Patch has been done.
   [*] New RSA-2048 private key has been saved to
       C:\Users\DoubleSine\source\repos\navicat-keygen\bin\x64-Release\RegPrivateKey.pem


   *******************************************************
   *           PATCH HAS BEEN DONE SUCCESSFULLY!         *
   *                  HAVE FUN AND ENJOY~                *
   *******************************************************
   ```

2. Then use `navicat-keygen.exe` to generate __snKey__ and __Activation Code__

   ```
   navicat-keygen.exe <-bin|-text> [-adv] <RSA-2048 Private Key File>
   ```

   * `<-bin|-text>`: Must be `-bin` or `-text`. 
  
     If `-bin` is specified, `navicat-keygen.exe` will finally generate `license_file`. It is used for Navicat old activation method only.

     If `-text` is specified, `navicat-keygen.exe` will finally generate a Base64-style string which is __Activation Code__. It is used for Navicat new activation method. 

     __This parameter must be specified.__

   * `[-adv]`: Enable advanced mode.

     __This parameter is optional.__ If specified, `navicat-keygen.exe` will ask you input Navicat product ID number, language signature numbers. It is for future use generally.

   * `<RSA-2048 Private Key File>`: The full path or relative path to an RSA-2048 private key file. The private key must be in PEM format.
     
     __This parameter must be specified.__

   __Example: (in cmd.exe)__

   ```console
   navicat-keygen.exe -text .\RegPrivateKey.pem
   ```

   You will be asked to select Navicat product, language and input major version number. After that an randomly generated __snKey__ will be given.

   ```
   ***************************************************
   *       navicat-keygen by @DoubleLabyrinth        *
   *                version: 16.0.7.0                *
   ***************************************************

   [*] Select Navicat product:
   0. DataModeler
   1. Premium
   2. MySQL
   3. PostgreSQL
   4. Oracle
   5. SQLServer
   6. SQLite
   7. MariaDB
   8. MongoDB
   9. ReportViewer

   (Input index)> 1

   [*] Select product language:
   0. English
   1. Simplified Chinese
   2. Traditional Chinese
   3. Japanese
   4. Polish
   5. Spanish
   6. French
   7. German
   8. Korean
   9. Russian
   10. Portuguese

   (Input index)> 0

   [*] Input major version number:
   (range: 11 ~ 16, default: 16)> 16

   [*] Serial number:
   NAVL-GFKA-T5SR-ZFTK

   [*] Your name:
   ```

   You can use this __snKey__ to activate your Navicat preliminarily.
     
   Then you will be asked to input `Your name` and `Your organization`. Just set them whatever you want, but not too long.

   ```
   [*] Your name: Double Sine
   [*] Your organization: PremiumSoft CyberTech Ltd.

   [*] Input request code (in Base64), input empty line to end:
   ```
     
   After that, you will be asked to input the request code. Now __DO NOT CLOSE KEYGEN__.

3. __Disconnect your network__ and open Navicat. Find and click `Registration`. Fill `Registration Key` by __snKey__ that the keygen gave and click `Activate`.

4. Generally online activation will failed and Navicat will ask you do `Manual Activation`, just choose it.

5. Copy your request code and paste it in the keygen. Input empty line to tell the keygen that your input ends.

   ```
   [*] Your name: Double Sine
   [*] Your organization: PremiumSoft CyberTech Ltd.

   [*] Input request code (in Base64), input empty line to end:
   CpgnfbIJGmAcxCuo/pAb8EeoS0audZn2NNemg6c3NPK/dWgb343IZQrFwoBZY6lpxE4Fq1BoNmCM75P03XpiXQ+hErcvFWk6iQPDCk/d4msf/AoprIqAMpXFoFLkeP0G93UIIEeBsUej8SrxdDgQDM585iPok5fUW+fTDCD1VICr7DBdL3c/69IxeIgiOQSuImdIQiM3/EOfDiFbAJL9vHW5LxFT8jj+8RPXehwPTBphpInmGdzxVZUZJwAGlXt7orrRbzafdeBjz6MnTajTcJP3SS2dBCiR33UScnyxYGEXdzv7+QLScTmCvI7gqg3Z8DMhroKMoHmy1AvC16FKVw==

   [*] Request Info:
   {"K":"NAVLGFKAT5SRZFTK", "DI":"7D48FCBD093C778879A1", "P":"WIN"}

   [*] Response Info:
   {"K":"NAVLGFKAT5SRZFTK","DI":"7D48FCBD093C778879A1","N":"Double Sine","O":"PremiumSoft CyberTech Ltd.","T":1644387294}

   [*] Activation Code:
   vwLGmQIWg/DtzHMcaKCDHAjTcBNbTo2VmNllphUSUMgGjgvL6v82ue+GqXB6M/qn48Rj4D4Joqqisr6UwMSclNmQxOQz4RftEpLtG6KBjDo4LM71qn9R/jWoZV5EoHPQkX5gzhO/D7GammrRGn2MV+zI6dJ4c4SBFNnNyjAeEqNzinrQwjB7lUVTlpHEe/SMrdCsGliPZQ/X+5ASbEsq3D8PZsjysJv98MIJrZvdTdznrRe8JzYP+8sbIPQMIX1UDmdyDpbpSl45N92OhO4htz1kFjUEfnrwY0GMOhdYHv/PfMI7RiQzkRyY7pLvX7muJ4dkA+CmMmwew3gy3MWjig==
   ```

6. Finally, you will get __Activation Code__ which looks like a Base64 string. Just copy it and paste it in Navicat `Manual Activation` window, then click `Activate`. If nothing wrong, activation should be done successfully.

