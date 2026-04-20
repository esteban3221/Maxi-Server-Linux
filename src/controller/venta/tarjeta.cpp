/*
<?xml version="1.0" encoding="UTF-8"?>
<interface>
  <requires lib="gtk" version="4.0"/>
  <template class="MetodoPagoTarjeta" parent="GtkBox">
    <property name="orientation">1</property>
    <property name="valign">3</property>
    <property name="halign">3</property>
    <property name="spacing">32</property>
    <child>
      <object class="GtkImage">
        <property name="icon-name">credit-card-symbolic</property>
        <property name="pixel-size">128</property>
        <style>
          <class name="dim-label"/>
        </style>
      </object>
    </child>
    <child>
      <object class="GtkBox">
        <property name="orientation">1</property>
        <property name="spacing">8</property>
        <child>
          <object class="GtkLabel">
            <property name="label">Terminal de Pago Activa</property>
            <style>
              <class name="title-1"/>
            </style>
          </object>
        </child>
        <child>
          <object class="GtkLabel">
            <property name="label">Por favor, siga las instrucciones en la pantalla de la terminal bancaria.</property>
            <property name="justify">2</property>
            <property name="wrap">true</property>
            <property name="max-width-chars">40</property>
            <style>
              <class name="body"/>
            </style>
          </object>
        </child>
      </object>
    </child>
    <child>
      <object class="GtkSpinner">
        <property name="spinning">true</property>
        <property name="width-request">48</property>
        <property name="height-request">48</property>
      </object>
    </child>
    <child>
      <object class="GtkButton" id="btn_cancel_tarjeta">
        <property name="label">Cancelar Transacción</property>
        <property name="halign">3</property>
        <style>
          <class name="destructive-action"/>
          <class name="pill"/>
        </style>
        <property name="margin-top">20</property>
      </object>
    </child>
  </template>
</interface>
*/