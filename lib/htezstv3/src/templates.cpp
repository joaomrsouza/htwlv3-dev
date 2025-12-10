/**
 * @file templates.cpp
 * @brief HTML Templates for EZSettings
 *
 * Description:
 *
 * This file contains the HTML templates used by the EZSettings web interface.
 * It includes the base template with CSS styling and the specific templates
 * for the settings form and save confirmation page.
 *
 * Depends On:
 * - htezstv3
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

#include "htezstv3.h"

String EZSettings::detail::getBaseTemplate()
{
  return R"(
    <!DOCTYPE html>
    <html>
      <head>
        <title>Settings</title>
        <meta name="viewport" content="width=device-width, initial-scale=1" />
        <style>
          * {
            font-family: Arial, Helvetica, sans-serif;
            margin: 0;
            padding: 0;
          }

          h1 {
            text-align: center;
            margin-bottom: 20px;
            background-color: #007bff;
            color: white;
            padding: 10px;
          }

          input,
          select {
            padding: 7px;
            border-radius: 5px;
            border: 1px solid #ccc;
          }

          input[type="checkbox"] {
            width: 20px;
            height: 20px;
          }

          a,
          button {
            padding: 10px;
            border-radius: 5px;
            border: 1px solid #ccc;
            background-color: #007bff;
            color: white;
            cursor: pointer;
            font-size: 16px;
            font-weight: bold;
            margin-top: 20px;
          }

          form,
          .config-content {
            display: flex;
            flex-direction: column;
            gap: 10px;
            padding: 20px;
          }

          .group {
            display: flex;
            flex-direction: column;
            gap: 3px;
          }

          .group:has(input[type="checkbox"]) {
            flex-direction: row-reverse;
            align-items: center;
            justify-content: start;
            gap: 10px;
          }

          .config-content:has(.group.group-enable > input:not(:checked))
            > .group:not(.group-enable) {
            display: none;
          }

          .group label {
            font-size: 14px;
            font-weight: bold;
          }

          details {
            border: 1px solid #ccc;
            border-radius: 5px;
            margin-bottom: 10px;
            padding: 10px;
            background-color: #f9f9f9;
          }

          summary {
            font-weight: bold;
            font-size: 16px;
            padding: 10px;
            margin: -10px;
            cursor: pointer;
            background-color: #007bff;
            color: white;
            border-radius: 5px;
            user-select: none;
          }

          summary:hover {
            background-color: #0056b3;
          }

          details[open] summary {
            border-radius: 5px 5px 0 0;
            margin-bottom: 10px;
          }

          .config-content {
            padding-top: 10px;
          }

          .center {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
          }
        </style>
      </head>
      <body>
        {PAGE_CONTENT}
      </body>
    </html>
  )";
}

String EZSettings::detail::getSettingsTemplate()
{
  return R"(
    <h1>Settings</h1>
    <form action="/settings/save" method="post">
      <details open>
        <summary>Board Configuration</summary>
        <div class="config-content">
          <div class="group">
            <label for="display-enable">
              Display Enable (current: {LABEL_DISPLAY_ENABLE})
            </label>
            <input
              {CHECKED_DISPLAY_ENABLE}
              type="checkbox"
              id="display-enable"
              name="display-enable"
            />
          </div>
          <div class="group">
            <label for="serial-enable">
              Serial Enable (current: {LABEL_SERIAL_ENABLE})
            </label>
            <input
              {CHECKED_SERIAL_ENABLE}
              type="checkbox"
              id="serial-enable"
              name="serial-enable"
            />
          </div>
          <div class="group">
            <label for="serial-speed">
              Serial Speed (current: {LABEL_SERIAL_SPEED})
            </label>
            <select
              id="serial-speed"
              name="serial-speed"
            >
              <option {SELECTED_SERIAL_SPEED-4800} value="4800">4800</option>
              <option {SELECTED_SERIAL_SPEED-9600} value="9600">9600</option>
              <option {SELECTED_SERIAL_SPEED-19200} value="19200">19200</option>
              <option {SELECTED_SERIAL_SPEED-38400} value="38400">38400</option>
              <option {SELECTED_SERIAL_SPEED-57600} value="57600">57600</option>
              <option {SELECTED_SERIAL_SPEED-115200} value="115200">115200</option>
            </select>
          </div>
        </div>
      </details>

      <details open>
        <summary>LoRa Configuration</summary>
        <div class="config-content">
          <div class="group group-enable">
            <label for="lora-enable">
              LoRa Enable (current: {LABEL_LORA_ENABLE})
            </label>
            <input
              {CHECKED_LORA_ENABLE}
              type="checkbox"
              id="lora-enable"
              name="lora-enable"
            />
          </div>
          <div class="group">
            <label for="lora-frequency">
              Frequency (current: {LABEL_LORA_FREQUENCY})
            </label>
            <select
              id="lora-frequency"
              name="lora-frequency"
            >
              <option {SELECTED_LORA_FREQUENCY-433000000} value="433000000">433 MHz</option>
              <option {SELECTED_LORA_FREQUENCY-470000000} value="470000000">470 MHz</option>
              <option {SELECTED_LORA_FREQUENCY-868000000} value="868000000">868 MHz</option>
              <option {SELECTED_LORA_FREQUENCY-915000000} value="915000000">915 MHz</option>
            </select>
          </div>
          <div class="group">
            <label for="lora-bandwidth">
              Bandwidth (current: {LABEL_LORA_BANDWIDTH})
            </label>
            <select
              id="lora-bandwidth"
              name="lora-bandwidth"
            >
              <option {SELECTED_LORA_BANDWIDTH-0} value="0">125 kHz</option>
              <option {SELECTED_LORA_BANDWIDTH-1} value="1">250 kHz</option>
              <option {SELECTED_LORA_BANDWIDTH-2} value="2">500 kHz</option>
              <option {SELECTED_LORA_BANDWIDTH-3} value="3">Reserved</option>
            </select>
          </div>
          <div class="group">
            <label for="lora-spreading-factor">
              Spreading Factor (current: {LABEL_LORA_SPREADING_FACTOR})
            </label>
            <select
              id="lora-spreading-factor"
              name="lora-spreading-factor"
              value="{CURRENT_LORA_SPREADING_FACTOR}"
            >
              <option {SELECTED_LORA_SPREADING_FACTOR-7} value="7">7</option>
              <option {SELECTED_LORA_SPREADING_FACTOR-8} value="8">8</option>
              <option {SELECTED_LORA_SPREADING_FACTOR-9} value="9">9</option>
              <option {SELECTED_LORA_SPREADING_FACTOR-10} value="10">10</option>
              <option {SELECTED_LORA_SPREADING_FACTOR-11} value="11">11</option>
              <option {SELECTED_LORA_SPREADING_FACTOR-12} value="12">12</option>
            </select>
          </div>
          <div class="group">
            <label for="lora-coding-rate">
              Coding Rate (current: {LABEL_LORA_CODING_RATE})
            </label>
            <select
              id="lora-coding-rate"
              name="lora-coding-rate"
              value="{CURRENT_LORA_CODING_RATE}"
            >
              <option {SELECTED_LORA_CODING_RATE-1} value="1">4/5</option>
              <option {SELECTED_LORA_CODING_RATE-2} value="2">4/6</option>
              <option {SELECTED_LORA_CODING_RATE-3} value="3">4/7</option>
              <option {SELECTED_LORA_CODING_RATE-4} value="4">4/8</option>
            </select>
          </div>
          <div class="group">
            <label for="lora-preamble-length">
              Preamble Length (current: {LABEL_LORA_PREAMBLE_LENGTH})
            </label>
            <select
              id="lora-preamble-length"
              name="lora-preamble-length"
              value="{CURRENT_LORA_PREAMBLE_LENGTH}"
            >
              <option {SELECTED_LORA_PREAMBLE_LENGTH-6} value="6">6</option>
              <option {SELECTED_LORA_PREAMBLE_LENGTH-8} value="8">8</option>
              <option {SELECTED_LORA_PREAMBLE_LENGTH-12} value="12">12</option>
              <option {SELECTED_LORA_PREAMBLE_LENGTH-16} value="16">16</option>
              <option {SELECTED_LORA_PREAMBLE_LENGTH-20} value="20">20</option>
            </select>
          </div>
          <div class="group">
            <label for="lora-fix-length-payload">
              Fix Length Payload (current: {LABEL_LORA_FIX_LENGTH_PAYLOAD})
            </label>
            <input
              {CHECKED_LORA_FIX_LENGTH_PAYLOAD}
              type="checkbox"
              id="lora-fix-length-payload"
              name="lora-fix-length-payload"
            />
          </div>
          <div class="group">
            <label for="lora-iq-inversion">
              IQ Inversion (current: {LABEL_LORA_IQ_INVERSION})
            </label>
            <input
              {CHECKED_LORA_IQ_INVERSION}
              type="checkbox"
              id="lora-iq-inversion"
              name="lora-iq-inversion"
            />
          </div>
          <div class="group">
            <label for="lora-tx-out-power">
              TX Out Power (current: {LABEL_LORA_TX_OUT_POWER})
            </label>
            <select
              id="lora-tx-out-power"
              name="lora-tx-out-power"
              value="{CURRENT_LORA_TX_OUT_POWER}"
            >
              <option {SELECTED_LORA_TX_OUT_POWER-0} value="0">0</option>
              <option {SELECTED_LORA_TX_OUT_POWER-1} value="1">1</option>
              <option {SELECTED_LORA_TX_OUT_POWER-2} value="2">2</option>
              <option {SELECTED_LORA_TX_OUT_POWER-3} value="3">3</option>
              <option {SELECTED_LORA_TX_OUT_POWER-4} value="4">4</option>
              <option {SELECTED_LORA_TX_OUT_POWER-5} value="5">5</option>
              <option {SELECTED_LORA_TX_OUT_POWER-6} value="6">6</option>
              <option {SELECTED_LORA_TX_OUT_POWER-7} value="7">7</option>
              <option {SELECTED_LORA_TX_OUT_POWER-8} value="8">8</option>
              <option {SELECTED_LORA_TX_OUT_POWER-9} value="9">9</option>
              <option {SELECTED_LORA_TX_OUT_POWER-10} value="10">10</option>
              <option {SELECTED_LORA_TX_OUT_POWER-11} value="11">11</option>
              <option {SELECTED_LORA_TX_OUT_POWER-12} value="12">12</option>
              <option {SELECTED_LORA_TX_OUT_POWER-13} value="13">13</option>
              <option {SELECTED_LORA_TX_OUT_POWER-14} value="14">14</option>
              <option {SELECTED_LORA_TX_OUT_POWER-15} value="15">15</option>
              <option {SELECTED_LORA_TX_OUT_POWER-16} value="16">16</option>
              <option {SELECTED_LORA_TX_OUT_POWER-17} value="17">17</option>
              <option {SELECTED_LORA_TX_OUT_POWER-18} value="18">18</option>
              <option {SELECTED_LORA_TX_OUT_POWER-19} value="19">19</option>
              <option {SELECTED_LORA_TX_OUT_POWER-20} value="20">20</option>
              <option {SELECTED_LORA_TX_OUT_POWER-21} value="21">21</option>
              <option {SELECTED_LORA_TX_OUT_POWER-22} value="22">22</option>
              <option {SELECTED_LORA_TX_OUT_POWER-23} value="23">23</option>
              <option {SELECTED_LORA_TX_OUT_POWER-24} value="24">24</option>
              <option {SELECTED_LORA_TX_OUT_POWER-25} value="25">25</option>
              <option {SELECTED_LORA_TX_OUT_POWER-26} value="26">26</option>
              <option {SELECTED_LORA_TX_OUT_POWER-27} value="27">27</option>
              <option {SELECTED_LORA_TX_OUT_POWER-28} value="28">28</option>
              <option {SELECTED_LORA_TX_OUT_POWER-29} value="29">29</option>
              <option {SELECTED_LORA_TX_OUT_POWER-30} value="30">30</option>
            </select>
          </div>
          <div class="group">
            <label for="lora-tx-timeout">
              TX Timeout (current: {VALUE_LORA_TX_TIMEOUT})
            </label>
            <input
              type="number"
              min="0"
              id="lora-tx-timeout"
              name="lora-tx-timeout"
              value="{VALUE_LORA_TX_TIMEOUT}"
            />
          </div>
          <div class="group">
            <label for="lora-rx-timeout">
              RX Timeout (current: {VALUE_LORA_RX_TIMEOUT})
            </label>
            <input
              type="number"
              min="0"
              id="lora-rx-timeout"
              name="lora-rx-timeout"
              value="{VALUE_LORA_RX_TIMEOUT}"
            />
          </div>
        </div>
      </details>

      <details open>
        <summary>WiFi - Client Configuration</summary>
        <div class="config-content">
          <div class="group group-enable">
            <label for="client-enable">
              Client Enable (current: {LABEL_CLIENT_ENABLE})
            </label>
            <input
              {CHECKED_CLIENT_ENABLE}
              type="checkbox"
              id="client-enable"
              name="client-enable"
            />
          </div>
          <div class="group">
            <label for="client-ssid">
              SSID (current: {VALUE_CLIENT_SSID})
            </label>
            <input
              id="client-ssid"
              name="client-ssid"
              value="{VALUE_CLIENT_SSID}"
            />
          </div>
          <div class="group">
            <label for="client-password">
              Password (current: {VALUE_CLIENT_PASSWORD})
            </label>
            <input
              type="password"
              id="client-password"
              name="client-password"
              value="{VALUE_CLIENT_PASSWORD}"
            />
          </div>
        </div>
      </details>

      <details open>
        <summary>WiFi - Server Configuration</summary>
        <div class="config-content">
          <div class="group group-enable">
            <label for="server-enable">
              Server Enable (current: {LABEL_SERVER_ENABLE})
            </label>
            <input
              {CHECKED_SERVER_ENABLE}
              type="checkbox"
              id="server-enable"
              name="server-enable"
            />
          </div>
          <div class="group">
            <label for="server-ssid">
              SSID (current: {VALUE_SERVER_SSID})
            </label>
            <input
              id="server-ssid"
              name="server-ssid"
              value="{VALUE_SERVER_SSID}"
            />
          </div>
          <div class="group">
            <label for="server-password">
              Password (current: {VALUE_SERVER_PASSWORD})
            </label>
            <input
              type="password"
              id="server-password"
              name="server-password"
              value="{VALUE_SERVER_PASSWORD}"
            />
          </div>
        </div>
      </details>
      <button type="submit">Save</button>
    </form>
  )";
}

String EZSettings::detail::getSaveTemplate()
{
  return R"(
    <h1>Settings saved successfully</h1>
    <div class="center">
      <p>Redirecting to settings page in 3 seconds...</p>
      <a href="/settings">Go back to settings</a>
    </div>
    <script>
      setTimeout(() => {
        window.location.href = "/settings";
      }, 3000);
    </script>
  )";
}
